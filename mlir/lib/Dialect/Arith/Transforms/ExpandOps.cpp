//===- ExpandOps.cpp - Pass to legalize Arith ops for LLVM lowering --===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Arith/Transforms/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/IR/BuiltinTypeInterfaces.h"
#include "mlir/IR/ImplicitLocOpBuilder.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/ADT/SmallVectorExtras.h"
#include <cstdint>

namespace mlir {
namespace arith {
#define GEN_PASS_DEF_ARITHEXPANDOPSPASS
#include "mlir/Dialect/Arith/Transforms/Passes.h.inc"
} // namespace arith
} // namespace mlir

using namespace mlir;

/// Create an integer or index constant.
static Value createConst(Location loc, Type type, int value,
                         PatternRewriter &rewriter) {
  auto attr = rewriter.getIntegerAttr(getElementTypeOrSelf(type), value);
  if (auto shapedTy = dyn_cast<ShapedType>(type)) {
    return rewriter.create<arith::ConstantOp>(
        loc, DenseElementsAttr::get(shapedTy, attr));
  }

  return rewriter.create<arith::ConstantOp>(loc, attr);
}

/// Create a float constant.
static Value createFloatConst(Location loc, Type type, APFloat value,
                              PatternRewriter &rewriter) {
  auto attr = rewriter.getFloatAttr(getElementTypeOrSelf(type), value);
  if (auto shapedTy = dyn_cast<ShapedType>(type)) {
    return rewriter.create<arith::ConstantOp>(
        loc, DenseElementsAttr::get(shapedTy, attr));
  }

  return rewriter.create<arith::ConstantOp>(loc, attr);
}

/// Creates shapedType using shape from cloneFrom and base type from cloneTo
static Type cloneToShapedType(Type cloneFrom, Type cloneTo) {
  if (auto shapedTy = dyn_cast<ShapedType>(cloneFrom)) {
    return shapedTy.clone(cloneTo);
  }
  return cloneTo;
}

namespace {

/// Expands CeilDivUIOp (n, m) into
///  n == 0 ? 0 : ((n-1) / m) + 1
struct CeilDivUIOpConverter : public OpRewritePattern<arith::CeilDivUIOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::CeilDivUIOp op,
                                PatternRewriter &rewriter) const final {
    Location loc = op.getLoc();
    Value a = op.getLhs();
    Value b = op.getRhs();
    Value zero = createConst(loc, a.getType(), 0, rewriter);
    Value compare =
        rewriter.create<arith::CmpIOp>(loc, arith::CmpIPredicate::eq, a, zero);
    Value one = createConst(loc, a.getType(), 1, rewriter);
    Value minusOne = rewriter.create<arith::SubIOp>(loc, a, one);
    Value quotient = rewriter.create<arith::DivUIOp>(loc, minusOne, b);
    Value plusOne = rewriter.create<arith::AddIOp>(loc, quotient, one);
    rewriter.replaceOpWithNewOp<arith::SelectOp>(op, compare, zero, plusOne);
    return success();
  }
};

/// Expands CeilDivSIOp (n, m) into
///   1) x = (m > 0) ? -1 : 1
///   2) (n*m>0) ? ((n+x) / m) + 1 : - (-n / m)
struct CeilDivSIOpConverter : public OpRewritePattern<arith::CeilDivSIOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::CeilDivSIOp op,
                                PatternRewriter &rewriter) const final {
    Location loc = op.getLoc();
    Type type = op.getType();
    Value a = op.getLhs();
    Value b = op.getRhs();
    Value plusOne = createConst(loc, type, 1, rewriter);
    Value zero = createConst(loc, type, 0, rewriter);
    Value minusOne = createConst(loc, type, -1, rewriter);
    // Compute x = (b>0) ? -1 : 1.
    Value compare =
        rewriter.create<arith::CmpIOp>(loc, arith::CmpIPredicate::sgt, b, zero);
    Value x = rewriter.create<arith::SelectOp>(loc, compare, minusOne, plusOne);
    // Compute positive res: 1 + ((x+a)/b).
    Value xPlusA = rewriter.create<arith::AddIOp>(loc, x, a);
    Value xPlusADivB = rewriter.create<arith::DivSIOp>(loc, xPlusA, b);
    Value posRes = rewriter.create<arith::AddIOp>(loc, plusOne, xPlusADivB);
    // Compute negative res: - ((-a)/b).
    Value minusA = rewriter.create<arith::SubIOp>(loc, zero, a);
    Value minusADivB = rewriter.create<arith::DivSIOp>(loc, minusA, b);
    Value negRes = rewriter.create<arith::SubIOp>(loc, zero, minusADivB);
    // Result is (a*b>0) ? pos result : neg result.
    // Note, we want to avoid using a*b because of possible overflow.
    // The case that matters are a>0, a==0, a<0, b>0 and b<0. We do
    // not particuliarly care if a*b<0 is true or false when b is zero
    // as this will result in an illegal divide. So `a*b<0` can be reformulated
    // as `(a<0 && b<0) || (a>0 && b>0)' or `(a<0 && b<0) || (a>0 && b>=0)'.
    // We pick the first expression here.
    Value aNeg =
        rewriter.create<arith::CmpIOp>(loc, arith::CmpIPredicate::slt, a, zero);
    Value aPos =
        rewriter.create<arith::CmpIOp>(loc, arith::CmpIPredicate::sgt, a, zero);
    Value bNeg =
        rewriter.create<arith::CmpIOp>(loc, arith::CmpIPredicate::slt, b, zero);
    Value bPos =
        rewriter.create<arith::CmpIOp>(loc, arith::CmpIPredicate::sgt, b, zero);
    Value firstTerm = rewriter.create<arith::AndIOp>(loc, aNeg, bNeg);
    Value secondTerm = rewriter.create<arith::AndIOp>(loc, aPos, bPos);
    Value compareRes =
        rewriter.create<arith::OrIOp>(loc, firstTerm, secondTerm);
    // Perform substitution and return success.
    rewriter.replaceOpWithNewOp<arith::SelectOp>(op, compareRes, posRes,
                                                 negRes);
    return success();
  }
};

/// Expands FloorDivSIOp (x, y) into
/// z = x / y
/// if (z * y != x && (x < 0) != (y < 0)) {
///   return  z - 1;
/// } else {
///   return z;
/// }
struct FloorDivSIOpConverter : public OpRewritePattern<arith::FloorDivSIOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::FloorDivSIOp op,
                                PatternRewriter &rewriter) const final {
    Location loc = op.getLoc();
    Type type = op.getType();
    Value a = op.getLhs();
    Value b = op.getRhs();

    Value quotient = rewriter.create<arith::DivSIOp>(loc, a, b);
    Value product = rewriter.create<arith::MulIOp>(loc, quotient, b);
    Value notEqualDivisor = rewriter.create<arith::CmpIOp>(
        loc, arith::CmpIPredicate::ne, a, product);
    Value zero = createConst(loc, type, 0, rewriter);

    Value aNeg =
        rewriter.create<arith::CmpIOp>(loc, arith::CmpIPredicate::slt, a, zero);
    Value bNeg =
        rewriter.create<arith::CmpIOp>(loc, arith::CmpIPredicate::slt, b, zero);

    Value signOpposite = rewriter.create<arith::CmpIOp>(
        loc, arith::CmpIPredicate::ne, aNeg, bNeg);
    Value cond =
        rewriter.create<arith::AndIOp>(loc, notEqualDivisor, signOpposite);

    Value minusOne = createConst(loc, type, -1, rewriter);
    Value quotientMinusOne =
        rewriter.create<arith::AddIOp>(loc, quotient, minusOne);

    rewriter.replaceOpWithNewOp<arith::SelectOp>(op, cond, quotientMinusOne,
                                                 quotient);
    return success();
  }
};

template <typename OpTy, arith::CmpIPredicate pred>
struct MaxMinIOpConverter : public OpRewritePattern<OpTy> {
public:
  using OpRewritePattern<OpTy>::OpRewritePattern;

  LogicalResult matchAndRewrite(OpTy op,
                                PatternRewriter &rewriter) const final {
    Value lhs = op.getLhs();
    Value rhs = op.getRhs();

    Value cmp = rewriter.create<arith::CmpIOp>(op.getLoc(), pred, lhs, rhs);
    rewriter.replaceOpWithNewOp<arith::SelectOp>(op, cmp, lhs, rhs);
    return success();
  }
};

template <typename OpTy, arith::CmpFPredicate pred>
struct MaximumMinimumFOpConverter : public OpRewritePattern<OpTy> {
public:
  using OpRewritePattern<OpTy>::OpRewritePattern;

  LogicalResult matchAndRewrite(OpTy op,
                                PatternRewriter &rewriter) const final {
    Value lhs = op.getLhs();
    Value rhs = op.getRhs();

    Location loc = op.getLoc();
    // If any operand is NaN, 'cmp' will be true (and 'select' returns 'lhs').
    static_assert(pred == arith::CmpFPredicate::UGT ||
                      pred == arith::CmpFPredicate::ULT,
                  "pred must be either UGT or ULT");
    Value cmp = rewriter.create<arith::CmpFOp>(loc, pred, lhs, rhs);
    Value select = rewriter.create<arith::SelectOp>(loc, cmp, lhs, rhs);

    // Handle the case where rhs is NaN: 'isNaN(rhs) ? rhs : select'.
    Value isNaN = rewriter.create<arith::CmpFOp>(loc, arith::CmpFPredicate::UNO,
                                                 rhs, rhs);
    rewriter.replaceOpWithNewOp<arith::SelectOp>(op, isNaN, rhs, select);
    return success();
  }
};

template <typename OpTy, arith::CmpFPredicate pred>
struct MaxNumMinNumFOpConverter : public OpRewritePattern<OpTy> {
public:
  using OpRewritePattern<OpTy>::OpRewritePattern;

  LogicalResult matchAndRewrite(OpTy op,
                                PatternRewriter &rewriter) const final {
    Value lhs = op.getLhs();
    Value rhs = op.getRhs();

    Location loc = op.getLoc();
    // If any operand is NaN, 'cmp' will be true (and 'select' returns 'lhs').
    static_assert(pred == arith::CmpFPredicate::UGT ||
                      pred == arith::CmpFPredicate::ULT,
                  "pred must be either UGT or ULT");
    Value cmp = rewriter.create<arith::CmpFOp>(loc, pred, lhs, rhs);
    Value select = rewriter.create<arith::SelectOp>(loc, cmp, lhs, rhs);

    // Handle the case where lhs is NaN: 'isNaN(lhs) ? rhs : select'.
    Value isNaN = rewriter.create<arith::CmpFOp>(loc, arith::CmpFPredicate::UNO,
                                                 lhs, lhs);
    rewriter.replaceOpWithNewOp<arith::SelectOp>(op, isNaN, rhs, select);
    return success();
  }
};

struct BFloat16ExtFOpConverter : public OpRewritePattern<arith::ExtFOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::ExtFOp op,
                                PatternRewriter &rewriter) const final {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);
    auto operand = op.getOperand();
    Type operandTy = operand.getType();
    Type resultTy = op.getType();
    Type operandETy = getElementTypeOrSelf(operandTy);
    Type resultETy = getElementTypeOrSelf(resultTy);

    if (!operandETy.isBF16() || !resultETy.isF32()) {
      return rewriter.notifyMatchFailure(op, "not a ext of bf16 to f32.");
    }

    Type i16Ty = b.getI16Type();
    Type i32Ty = b.getI32Type();
    if (auto shapedTy = dyn_cast<ShapedType>(operandTy)) {
      i16Ty = shapedTy.clone(i16Ty);
      i32Ty = shapedTy.clone(i32Ty);
    }

    Value bitcast = b.create<arith::BitcastOp>(i16Ty, operand);
    Value exti = b.create<arith::ExtUIOp>(i32Ty, bitcast);

    Value c16 = createConst(op.getLoc(), i32Ty, 16, rewriter);
    Value shl = b.create<arith::ShLIOp>(exti, c16);
    Value result = b.create<arith::BitcastOp>(resultTy, shl);

    rewriter.replaceOp(op, result);
    return success();
  }
};

struct BFloat16TruncFOpConverter : public OpRewritePattern<arith::TruncFOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::TruncFOp op,
                                PatternRewriter &rewriter) const final {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);
    auto operand = op.getOperand();
    Type operandTy = operand.getType();
    Type resultTy = op.getType();
    Type operandETy = getElementTypeOrSelf(operandTy);
    Type resultETy = getElementTypeOrSelf(resultTy);

    if (!operandETy.isF32() || !resultETy.isBF16()) {
      return rewriter.notifyMatchFailure(op, "not a trunc of f32 to bf16.");
    }

    if (op.getRoundingmodeAttr()) {
      return rewriter.notifyMatchFailure(
          op, "only applicable to default rounding mode.");
    }

    Type i16Ty = b.getI16Type();
    Type i32Ty = b.getI32Type();
    Type f32Ty = b.getF32Type();
    if (auto shapedTy = dyn_cast<ShapedType>(operandTy)) {
      i16Ty = shapedTy.clone(i16Ty);
      i32Ty = shapedTy.clone(i32Ty);
      f32Ty = shapedTy.clone(f32Ty);
    }

    // Algorithm borrowed from this excellent code:
    // https://github.com/pytorch/pytorch/blob/e1502c0cdbfd17548c612f25d5a65b1e4b86224d/c10/util/BFloat16.h#L60-L79
    // There is a magic idea there, to let the addition of the rounding_bias to
    // the mantissa simply overflow into the exponent bits. It's a bit of an
    // aggressive, obfuscating optimization, but it is well-tested code, and it
    // results in more concise and efficient IR.
    // The case of NaN is handled separately (see isNaN and the final select).
    // The case of infinities is NOT handled separately, which deserves an
    // explanation. As the encoding of infinities has zero mantissa, the
    // rounding-bias addition never carries into the exponent so that just gets
    // truncated away, and as bfloat16 and float32 have the same number of
    // exponent bits, that simple truncation is the desired outcome for
    // infinities.
    Value isNan =
        b.create<arith::CmpFOp>(arith::CmpFPredicate::UNE, operand, operand);
    // Constant used to make the rounding bias.
    Value c7FFF = createConst(op.getLoc(), i32Ty, 0x7fff, rewriter);
    // Constant used to generate a quiet NaN.
    Value c7FC0_i16 = createConst(op.getLoc(), i16Ty, 0x7fc0, rewriter);
    // Small constants used to address bits.
    Value c16 = createConst(op.getLoc(), i32Ty, 16, rewriter);
    Value c1 = createConst(op.getLoc(), i32Ty, 1, rewriter);
    // Reinterpret the input f32 value as bits.
    Value bitcast = b.create<arith::BitcastOp>(i32Ty, operand);
    // Read bit 16 as a value in {0,1}.
    Value bit16 =
        b.create<arith::AndIOp>(b.create<arith::ShRUIOp>(bitcast, c16), c1);
    // Determine the rounding bias to add as either 0x7fff or 0x8000 depending
    // on bit 16, implementing the tie-breaking "to nearest even".
    Value roundingBias = b.create<arith::AddIOp>(bit16, c7FFF);
    // Add the rounding bias. Generally we want this to be added to the
    // mantissa, but nothing prevents this to from carrying into the exponent
    // bits, which would feel like a bug, but this is the magic trick here:
    // when that happens, the mantissa gets reset to zero and the exponent
    // gets incremented by the carry... which is actually exactly what we
    // want.
    Value biased = b.create<arith::AddIOp>(bitcast, roundingBias);
    // Now that the rounding-bias has been added, truncating the low bits
    // yields the correctly rounded result.
    Value biasedAndShifted = b.create<arith::ShRUIOp>(biased, c16);
    Value normalCaseResult_i16 =
        b.create<arith::TruncIOp>(i16Ty, biasedAndShifted);
    // Select either the above-computed result, or a quiet NaN constant
    // if the input was NaN.
    Value select =
        b.create<arith::SelectOp>(isNan, c7FC0_i16, normalCaseResult_i16);
    Value result = b.create<arith::BitcastOp>(resultTy, select);
    rewriter.replaceOp(op, result);
    return success();
  }
};

/// In this implementation of extf we take advantage of some key patterns we
/// notice between the binary representation of an F4E2M1 value and its
/// corresponding value in F32.
///
/// Note: x is sign bit
/// | Binary | F4E2M1 | f32[23:32]
/// | x000   | 0.0    | x000 0000 00
/// | x001   | 0.5    | x011 1111 00
/// | x010   | 1.0    | x011 1111 10
/// | x011   | 1.5    | x011 1111 11
/// | x100   | 2.0    | x010 0000 00
/// | x101   | 3.0    | x010 0000 01
/// | x110   | 4.0    | x010 0000 10
/// | x111   | 6.0    | x010 0000 11
///
/// 1) There are only two versions of bits [25:31] in the f32 result
///    F4E2M1 bits[2:3] decide whether:
///       - F32 bits[25:31] = 0011 1111
///       - F32 bits[25:31] = 0010 0000
///     Exception is zero where
///       - F32 bits[25:31] = 0000 0000
///
/// 2) F4E2M1 bits[1:2] = F32 bits[23:24]
///    Exception is 0.5 where
///       - F4E2M1 bits[1:2] = 01, F32 bits[23:24] = 00
///
/// 3) F4E2M1 bits[4] = F32 bits[32] (sign bits are equal)
///
/// 4) F32 bits[1:22] = 0
struct F4E2M1ExtFOpConverter : public OpRewritePattern<arith::ExtFOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::ExtFOp op,
                                PatternRewriter &rewriter) const final {
    Location loc = op.getLoc();
    ImplicitLocOpBuilder b(loc, rewriter);
    Value operand = op.getOperand();
    Type operandTy = operand.getType();
    Type resultTy = op.getType();
    Type operandETy = getElementTypeOrSelf(operandTy);
    Type resultETy = getElementTypeOrSelf(resultTy);

    if (!isa<Float4E2M1FNType>(operandETy))
      return rewriter.notifyMatchFailure(op, "not a ext of F4E2M1FN");

    Type f32Ty = cloneToShapedType(operandTy, b.getF32Type());
    Type i4Ty = cloneToShapedType(operandTy, b.getI4Type());
    Type i32Ty = cloneToShapedType(operandTy, b.getI32Type());
    Value i4Bits = b.create<arith::BitcastOp>(i4Ty, operand);

    Value c0x0 = createConst(loc, i4Ty, 0x0, rewriter);
    Value c0x1 = createConst(loc, i4Ty, 0x1, rewriter);
    Value c0x2 = createConst(loc, i4Ty, 0x2, rewriter);
    Value c0x4 = createConst(loc, i4Ty, 0x4, rewriter);

    // Set last Exponent bit and Mantissa.
    Value c0x00000014 = createConst(loc, i32Ty, 0x14, rewriter);
    Value bits1To24 = b.create<arith::ShLIOp>(i4Bits, c0x2);
    Value isHalf =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::eq, i4Bits, c0x1);
    bits1To24 = b.create<arith::SelectOp>(isHalf, c0x0, bits1To24);
    bits1To24 = b.create<arith::ExtUIOp>(i32Ty, bits1To24);
    bits1To24 = b.create<arith::ShLIOp>(bits1To24, c0x00000014);

    // Set first 7 bits of Exponent.
    Value zeroExpBits = createConst(loc, i32Ty, 0x00000000, rewriter);
    Value highExpBits = createConst(loc, i32Ty, 0x40000000, rewriter);
    Value lowExpBits = createConst(loc, i32Ty, 0x3f000000, rewriter);
    Value useLargerExp =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::uge, i4Bits, c0x4);
    Value bits25To31 =
        b.create<arith::SelectOp>(useLargerExp, highExpBits, lowExpBits);
    Value zeroExp =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::eq, i4Bits, c0x0);
    bits25To31 = b.create<arith::SelectOp>(zeroExp, zeroExpBits, bits25To31);

    // Set sign.
    Value c0x80000000 = createConst(loc, i32Ty, 0x80000000, rewriter);
    Value c0x8 = createConst(loc, i4Ty, 0x8, rewriter);
    Value negative =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::uge, i4Bits, c0x8);
    Value bit32 = b.create<arith::SelectOp>(negative, c0x80000000, zeroExpBits);

    // Add segments together.
    Value bits1To31 = b.create<arith::AddIOp>(bits1To24, bits25To31);
    Value bits1To32 = b.create<arith::AddIOp>(bits1To31, bit32);
    Value result = b.create<arith::BitcastOp>(f32Ty, bits1To32);
    if (!isa<Float32Type>(resultETy))
      result = b.create<arith::TruncFOp>(resultTy, result);

    rewriter.replaceOp(op, result);
    return success();
  }
};

struct F8E5M2ExtFOpConverter : public OpRewritePattern<arith::ExtFOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::ExtFOp op,
                                PatternRewriter &rewriter) const final {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);
    auto operand = op.getOperand();
    Type operandTy = operand.getType();
    Type resultTy = op.getType();
    Type operandETy = getElementTypeOrSelf(operandTy);
    Type resultETy = getElementTypeOrSelf(resultTy);

    // Match only f8E5M2 → f32 for now
    if (!llvm::isa<Float8E5M2Type>(operandETy) || !resultETy.isF32()) {
      return rewriter.notifyMatchFailure(op, "not a ext of f8E5M2 to f32.");
    }

    // Integer and float shaped types matching the input shape
    Type i8Ty = b.getI8Type();
    Type i32Ty = b.getI32Type();
    Type f32Ty = b.getF32Type();
    if (auto shapedTy = dyn_cast<ShapedType>(operandTy)) {
      i8Ty = shapedTy.clone(i8Ty);
      i32Ty = shapedTy.clone(i32Ty);
      f32Ty = shapedTy.clone(f32Ty);
    }

    // Bitcast fp8 to raw uint8
    Value bits = b.create<arith::BitcastOp>(i8Ty, operand);
    // Zero-extend to 32 bits
    Value bits32 = b.create<arith::ExtUIOp>(i32Ty, bits);

    // Extract sign (bit 7) → move to f32 sign position (bit 31)
    Value sign = b.create<arith::ShRUIOp>(
        bits32, createConst(op.getLoc(), i32Ty, 7, rewriter));
    sign = b.create<arith::AndIOp>(
        sign, createConst(op.getLoc(), i32Ty, 0x1, rewriter));

    // Extract exponent (bits 2–6) → move to f32 exponent position (bits 23–30)
    Value e5m2_exponent = b.create<arith::ShRUIOp>(
        bits32, createConst(op.getLoc(), i32Ty, 2, rewriter));
    e5m2_exponent = b.create<arith::AndIOp>(
        e5m2_exponent, createConst(op.getLoc(), i32Ty, 0x1F, rewriter));

    // Extract mantissa (bits 0–1)
    Value e5m2_mantissa = b.create<arith::AndIOp>(
        bits32,
        createConst(op.getLoc(), i32Ty, 0x3, rewriter)); // 0b11 mask for 2 bits

    // Bias exponent: f8E5M2 has a bias of 15, so we need to subtract 15
    Value exponent = b.create<arith::SubIOp>(
        e5m2_exponent, createConst(op.getLoc(), i32Ty, 15, rewriter));
    Value float_exponent = b.create<arith::AddIOp>(
        exponent, createConst(op.getLoc(), i32Ty, 127, rewriter));

    // Special case handling for NaNs, Infs, subnormals
    // Subnormal handling
    // if (e5m2_mantissa >= 0x2)
    Value isSubnormal =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sge, e5m2_mantissa,
                                createConst(op.getLoc(), i32Ty, 0x2, rewriter));
    // result = sign << 31 | (float_exponent) << 23 | (e5m2_mantissa & 0x1) <<
    // (23 - 1);
    Value subnormalResult = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 31, rewriter)),
        b.create<arith::OrIOp>(
            b.create<arith::ShLIOp>(
                float_exponent, createConst(op.getLoc(), i32Ty, 23, rewriter)),
            b.create<arith::ShLIOp>(
                b.create<arith::AndIOp>(
                    e5m2_mantissa,
                    createConst(op.getLoc(), i32Ty, 0x1, rewriter)),
                createConst(op.getLoc(), i32Ty, 22, rewriter))));

    // if (e5m2_mantissa == 0x1)
    Value isSubnormal2 =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::eq, e5m2_mantissa,
                                createConst(op.getLoc(), i32Ty, 0x1, rewriter));
    // result = sign << 31 | (float_exponent - 1) << 23;
    Value subnormalResult2 = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 31, rewriter)),
        b.create<arith::ShLIOp>(
            b.create<arith::SubIOp>(
                float_exponent, createConst(op.getLoc(), i32Ty, 1, rewriter)),
            createConst(op.getLoc(), i32Ty, 23, rewriter)));

    // Is normal if (e5m2_exponent > 0)
    Value isNormal =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sgt, e5m2_exponent,
                                createConst(op.getLoc(), i32Ty, 0, rewriter));

    // else nan
    Value NaN = createConst(op.getLoc(), i32Ty, 0x7FC00000, rewriter);

    // Combine sign | exponent | mantissa
    Value normalResult = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 31, rewriter)),
        b.create<arith::OrIOp>(
            b.create<arith::ShLIOp>(
                float_exponent, createConst(op.getLoc(), i32Ty, 23, rewriter)),
            b.create<arith::ShLIOp>(
                e5m2_mantissa, createConst(op.getLoc(), i32Ty, 21, rewriter))));

    // Select the appropriate result based on the conditions
    Value result = b.create<arith::SelectOp>(
        isNormal, normalResult,
        b.create<arith::SelectOp>(
            isSubnormal, subnormalResult,
            b.create<arith::SelectOp>(isSubnormal2, subnormalResult2, NaN)));

    // Bitcast to f32
    result = b.create<arith::BitcastOp>(f32Ty, result);

    rewriter.replaceOp(op, result);
    return success();
  }
};

struct F8E5M2TruncFOpConverter : public OpRewritePattern<arith::TruncFOp> {
  using OpRewritePattern::OpRewritePattern;

  LogicalResult matchAndRewrite(arith::TruncFOp op,
                                PatternRewriter &rewriter) const final {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);
    Value operand = op.getOperand();
    Type operandTy = operand.getType();
    Type operandETy = getElementTypeOrSelf(operandTy);
    Type resultTy = op.getType();
    Type resultETy = getElementTypeOrSelf(resultTy);

    if (!isa<Float8E5M2Type>(resultETy)) {
      return rewriter.notifyMatchFailure(op, "not a truncf to fp8e5m2");
    }

    if (op.getRoundingmodeAttr()) {
      return rewriter.notifyMatchFailure(
          op, "only applicable to default rounding mode.");
    }

    Type i8Ty = b.getI8Type();
    Type i32Ty = b.getI32Type();
    Type f32Ty = b.getF32Type();

    if (auto shapedTy = mlir::dyn_cast<ShapedType>(operandTy)) {
      i8Ty = shapedTy.clone(i8Ty);
      i32Ty = shapedTy.clone(i32Ty);
      f32Ty = shapedTy.clone(f32Ty);
    }

    // Normalize to f32
    if (operandETy.getIntOrFloatBitWidth() < 32) {
      operand = b.create<arith::ExtFOp>(f32Ty, operand, op.getFastmathAttr());
    } else if (operandETy.getIntOrFloatBitWidth() > 32) {
      operand = b.create<arith::TruncFOp>(
          f32Ty, operand, op.getRoundingmodeAttr(), op.getFastmathAttr());
    }

    // Bitcast f32 to i32 for bit manipulations
    Value bits = b.create<arith::BitcastOp>(i32Ty, operand);

    // Extract sign bit (bit 31)
    Value sign = b.create<arith::ShRUIOp>(
        bits, createConst(op.getLoc(), i32Ty, 31, rewriter));
    sign = b.create<arith::AndIOp>(
        sign, createConst(op.getLoc(), i32Ty, 0x1, rewriter));

    // Extract exponent bits (bits 30:23)
    Value exponent = b.create<arith::ShRUIOp>(
        bits, createConst(op.getLoc(), i32Ty, 23, rewriter));
    exponent = b.create<arith::AndIOp>(
        exponent, createConst(op.getLoc(), i32Ty, 0xFF, rewriter));

    // Compute unbiased exponent (exponent - 127)
    Value exponentBias = createConst(op.getLoc(), i32Ty, 127, rewriter);
    Value unbiasedExp = b.create<arith::SubIOp>(exponent, exponentBias);

    // Extract mantissa bits (bits 22:0)
    Value mantissa = b.create<arith::AndIOp>(
        bits, createConst(op.getLoc(), i32Ty, 0x7FFFFF, rewriter));

    // Add fp8 bias (15)
    Value fp8Bias = createConst(op.getLoc(), i32Ty, 15, rewriter);
    Value fp8Exp = b.create<arith::AddIOp>(unbiasedExp, fp8Bias);

    // Prepare mantissa for rounding:
    // We need to reduce mantissa from 23 bits → 2 bits mantissa in fp8.
    // To round to nearest, shift mantissa right by 21 (23 - 2)
    Value mantissaShift = createConst(op.getLoc(), i32Ty, 21, rewriter);
    Value mantissaTruncated = b.create<arith::ShRUIOp>(mantissa, mantissaShift);

    Value e5m2_mantissa = b.create<arith::AndIOp>(
        mantissaTruncated,
        createConst(op.getLoc(), i32Ty, 0x3, rewriter)); // 0b11 mask for 2 bits

    // Compose final fp8 bits: sign (bit7), expFinal (bits 6:2), mantissaFinal
    // (bits 1:0)
    Value signShifted = b.create<arith::ShLIOp>(
        sign, createConst(op.getLoc(), i32Ty, 7, rewriter));
    Value expShifted = b.create<arith::ShLIOp>(
        fp8Exp, createConst(op.getLoc(), i32Ty, 2, rewriter));
    Value resultInt = b.create<arith::OrIOp>(signShifted, expShifted);
    resultInt = b.create<arith::OrIOp>(resultInt, e5m2_mantissa);

    // Subnormal cases
    // if (e5m2_exponent > 31)
    Value isSubnormal =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sgt, fp8Exp,
                                createConst(op.getLoc(), i32Ty, 31, rewriter));
    // return sign << 7 | 0x7C;
    Value subnormalResult = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 7, rewriter)),
        createConst(op.getLoc(), i32Ty, 0x7C, rewriter) // 0b01111100
    );
    // if ((e5m2_exponent >= -1) && (e5m2_exponent <= 0))
    Value isSubnormal2 = b.create<arith::AndIOp>(
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sge, fp8Exp,
                                createConst(op.getLoc(), i32Ty, -1, rewriter)),
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sle, fp8Exp,
                                createConst(op.getLoc(), i32Ty, 0, rewriter)));
    // uint8_t shift_bits = (2 + e5m2_exponent);
    // uint8_t e5m2_mantissa = (mantissa >> (24 - shift_bits)) & (0x3 >> (0 -
    // e5m2_exponent)); return sign << 7 | 0x00 | e5m2_mantissa;

    Value shiftBits = b.create<arith::AddIOp>(
        createConst(op.getLoc(), i32Ty, 2, rewriter), fp8Exp);
    Value mantissaShift2 = b.create<arith::SubIOp>(
        createConst(op.getLoc(), i32Ty, 24, rewriter), shiftBits);
    Value e5m2_mantissa2 = b.create<arith::AndIOp>(
        b.create<arith::ShRUIOp>(mantissa, mantissaShift2),
        b.create<arith::ShRUIOp>(
            createConst(op.getLoc(), i32Ty, 0x3,
                        rewriter), // 0b11 mask for 2 bits
            b.create<arith::SubIOp>(
                createConst(op.getLoc(), i32Ty, 0, rewriter), fp8Exp)));
    Value subnormalResult2 = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 7, rewriter)),
        b.create<arith::OrIOp>(createConst(op.getLoc(), i32Ty, 0x00, rewriter),
                               e5m2_mantissa2));

    // if (e5m2_exponent < -1)
    Value isZero =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::slt, fp8Exp,
                                createConst(op.getLoc(), i32Ty, -1, rewriter));
    // return sign << 7 | 0x00;
    Value zeroResult = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 7, rewriter)),
        createConst(op.getLoc(), i32Ty, 0x00, rewriter));

    // Select the appropiate result based on the conditions
    Value finalResult = b.create<arith::SelectOp>(
        isSubnormal, subnormalResult,
        b.create<arith::SelectOp>(
            isSubnormal2, subnormalResult2,
            b.create<arith::SelectOp>(isZero, zeroResult, resultInt)));

    // Truncate to i8 and bitcast to fp8e5m2
    Value resultI8 = b.create<arith::TruncIOp>(i8Ty, finalResult);
    Value result = b.create<arith::BitcastOp>(resultTy, resultI8);

    rewriter.replaceOp(op, result);
    return success();
  }
};

struct F8E4M3FNExtFOpConverter : public OpRewritePattern<arith::ExtFOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::ExtFOp op,
                                PatternRewriter &rewriter) const final {

    ImplicitLocOpBuilder b(op.getLoc(), rewriter);
    auto operand = op.getOperand();
    Type operandTy = operand.getType();
    Type resultTy = op.getType();
    Type operandETy = getElementTypeOrSelf(operandTy);
    Type resultETy = getElementTypeOrSelf(resultTy);

    // Match only f8E4M3 → f32 for now
    if (!llvm::isa<Float8E4M3FNType>(operandETy) || !resultETy.isF32()) {
      return rewriter.notifyMatchFailure(op, "not a ext of f8E4M3 to f32.");
    }

    // Integer and float shaped types matching the input shape
    Type i8Ty = b.getI8Type();
    Type i32Ty = b.getI32Type();
    Type f32Ty = b.getF32Type();
    if (auto shapedTy = dyn_cast<ShapedType>(operandTy)) {
      i8Ty = shapedTy.clone(i8Ty);
      i32Ty = shapedTy.clone(i32Ty);
      f32Ty = shapedTy.clone(f32Ty);
    }

    // Bitcast fp8 to raw uint8
    Value bits = b.create<arith::BitcastOp>(i8Ty, operand);
    // Zero-extend to 32 bits
    Value bits32 = b.create<arith::ExtUIOp>(i32Ty, bits);

    // Extract sign
    Value sign = b.create<arith::ShRUIOp>(
        bits32, createConst(op.getLoc(), i32Ty, 7, rewriter));
    sign = b.create<arith::AndIOp>(
        sign, createConst(op.getLoc(), i32Ty, 0x1, rewriter));

    // extract exponent
    Value e4m3_exponent = b.create<arith::ShRUIOp>(
        bits32, createConst(op.getLoc(), i32Ty, 3, rewriter));
    e4m3_exponent = b.create<arith::AndIOp>(
        e4m3_exponent, createConst(op.getLoc(), i32Ty, 0xF, rewriter));

    // extract mantissa
    Value rounding_bias = createConst(op.getLoc(), i32Ty, 0x80000, rewriter);
    Value mantissa = b.create<arith::AddIOp>(bits32, rounding_bias);
    Value e4m3_mantissa = b.create<arith::AndIOp>(
        mantissa, createConst(op.getLoc(), i32Ty, 0x7, rewriter));

    // bias exponent
    Value exponent = b.create<arith::SubIOp>(
        e4m3_exponent, createConst(op.getLoc(), i32Ty, 7, rewriter));
    Value float_exponent = b.create<arith::AddIOp>(
        exponent, createConst(op.getLoc(), i32Ty, 127, rewriter));

    // put everything together (normal number) e4m3_exponent > 0
    Value isNormal =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sgt, e4m3_exponent,
                                createConst(op.getLoc(), i32Ty, 0, rewriter));

    Value result = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 31, rewriter)),
        b.create<arith::OrIOp>(
            b.create<arith::ShLIOp>(
                float_exponent, createConst(op.getLoc(), i32Ty, 23, rewriter)),
            b.create<arith::ShLIOp>(
                e4m3_mantissa, createConst(op.getLoc(), i32Ty, 20, rewriter))));

    // sub-normal numbers handling (e4m3_matissa >= 0x4)
    Value isSubnormal1 =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sge, e4m3_mantissa,
                                createConst(op.getLoc(), i32Ty, 0x4, rewriter));

    Value resultSubnormal1 = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 31, rewriter)),
        b.create<arith::OrIOp>(
            b.create<arith::ShLIOp>(
                float_exponent, createConst(op.getLoc(), i32Ty, 23, rewriter)),
            b.create<arith::AndIOp>(
                createConst(op.getLoc(), i32Ty, 0x3, rewriter),
                b.create<arith::ShLIOp>(
                    e4m3_mantissa,
                    createConst(op.getLoc(), i32Ty, 21, rewriter)))));

    // else if e4m3_mantissa > 0x1
    Value isSubnormal2 =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sgt, e4m3_mantissa,
                                createConst(op.getLoc(), i32Ty, 0x1, rewriter));

    Value resultSubormal2 = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 31, rewriter)),
        b.create<arith::OrIOp>(
            b.create<arith::ShLIOp>(
                b.create<arith::SubIOp>(
                    float_exponent,
                    createConst(op.getLoc(), i32Ty, 1, rewriter)),
                createConst(op.getLoc(), i32Ty, 23, rewriter)),
            b.create<arith::AndIOp>(
                createConst(op.getLoc(), i32Ty, 0x1, rewriter),
                b.create<arith::ShLIOp>(
                    e4m3_mantissa,
                    createConst(op.getLoc(), i32Ty, 22, rewriter)))));

    // else if e4m3_mantissa == 0x1
    Value isSubnormal3 =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::eq, e4m3_mantissa,
                                createConst(op.getLoc(), i32Ty, 0x1, rewriter));

    Value resultSubnormal3 = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 31, rewriter)),
        b.create<arith::ShLIOp>(
            b.create<arith::SubIOp>(
                float_exponent, createConst(op.getLoc(), i32Ty, 2, rewriter)),
            createConst(op.getLoc(), i32Ty, 23, rewriter)));

    // else Zero
    Value resultZero = b.create<arith::ShLIOp>(
        sign, createConst(op.getLoc(), i32Ty, 31, rewriter));

    // Compute final result
    result = b.create<arith::SelectOp>(
        isNormal, result,
        b.create<arith::SelectOp>(
            isSubnormal1, resultSubnormal1,
            b.create<arith::SelectOp>(
                isSubnormal2, resultSubormal2,
                b.create<arith::SelectOp>(isSubnormal3, resultSubnormal3,
                                          resultZero))));

    // Bitcast to f32
    result = b.create<arith::BitcastOp>(f32Ty, result);

    rewriter.replaceOp(op, result);
    return success();
  }
};

/// Conversion from F32 to F4E2M1 according to the OCP Spec:
/// www.opencompute.org/documents/ocp-microscaling-formats-mx-v1-0-spec-final-pdf
///
/// The spec requires us to perform Round to Nearest, Ties to Even.
///
/// This means that after rounding, we should break ties by choosing the option
/// which results in a mantissa of 0 in the least significant digit.
///
/// Table of representable values in F4E2M1:
///
/// Note: x is sign bit
/// | Binary | F4E2M1 | F32[23:32]
/// | x000   | 0.0    | x000 0000 00
/// | x001   | 0.5    | x011 1111 00
/// | x010   | 1.0    | x011 1111 10
/// | x011   | 1.5    | x011 1111 11
/// | x100   | 2.0    | x010 0000 00
/// | x101   | 3.0    | x010 0000 01
/// | x110   | 4.0    | x010 0000 10
/// | x111   | 6.0    | x010 0000 11
///
/// Conversion procedure:
///   Step 1: Clamp to representable bounds.
///   Step 2: Convert exponent by adjusting bias.
///   Step 3: Set mantissa to first bit.
///   Step 4: Special consideration for subnormal and zero exponent.
///   Step 5: Round up if necessary, if mantissa[1:] greater than 1000000 or
///   subnormal.
struct F4E2M1TruncFOpConverter : public OpRewritePattern<arith::TruncFOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::TruncFOp op,
                                PatternRewriter &rewriter) const final {
    Location loc = op.getLoc();
    ImplicitLocOpBuilder b(loc, rewriter);
    Value operand = op.getOperand();
    Type operandTy = operand.getType();
    Type resultTy = op.getType();
    Type operandETy = getElementTypeOrSelf(operandTy);
    Type resultETy = getElementTypeOrSelf(resultTy);

    Type i4Ty = cloneToShapedType(operandTy, b.getI4Type());
    Type i8Ty = cloneToShapedType(operandTy, b.getI8Type());
    Type i32Ty = cloneToShapedType(operandTy, b.getI32Type());
    Type f32Ty = cloneToShapedType(operandTy, b.getF32Type());

    if (!isa<Float32Type>(operandETy))
      operand = b.create<arith::ExtFOp>(f32Ty, operand);
    if (!isa<Float4E2M1FNType>(resultETy))
      return rewriter.notifyMatchFailure(op, "not a trunc of F4E2M1FN");

    Value c0x1 = createConst(loc, i4Ty, 1, rewriter);
    Value c0x3 = createConst(loc, i4Ty, 3, rewriter);
    Value c0x00000016 = createConst(loc, i32Ty, 22, rewriter);
    Value c0x00 = createConst(loc, i8Ty, 0x00, rewriter);
    Value c0xff = createConst(loc, i8Ty, 0xff, rewriter);
    Value zeroExpBits = createConst(loc, i32Ty, 0, rewriter);

    // Step 0: Clamp to bounds.
    Value cHigherBound = createFloatConst(loc, f32Ty, APFloat(6.0f), rewriter);
    Value cLowerBound = createFloatConst(loc, f32Ty, APFloat(-6.0f), rewriter);
    Value operandClamped = b.create<arith::MinNumFOp>(cHigherBound, operand);
    operandClamped = b.create<arith::MaxNumFOp>(cLowerBound, operandClamped);
    Value f32Bits = b.create<arith::BitcastOp>(i32Ty, operandClamped);

    // Step 1: Set sign bit.
    Value cF32ExpManWidth = createConst(loc, i32Ty, 31, rewriter);
    Value f32Sign = b.create<arith::ShRUIOp>(f32Bits, cF32ExpManWidth);
    Value f4Sign = b.create<arith::TruncIOp>(i4Ty, f32Sign);
    Value f4Bits = b.create<arith::ShLIOp>(f4Sign, c0x3);

    // Step 2: Convert exponent by adjusting bias.
    Value biasAdjustment = createConst(loc, i32Ty, 0x7e, rewriter);
    Value cF4MantissaWidth = c0x1;                                   // 1
    Value cF32MantissaWidth = createConst(loc, i32Ty, 23, rewriter); // 23
    Value f32SignExp = b.create<arith::ShRUIOp>(f32Bits, cF32MantissaWidth);
    Value biasAdjustedSignExp =
        b.create<arith::SubIOp>(f32SignExp, biasAdjustment);
    Value f4Exp = b.create<arith::TruncIOp>(i4Ty, biasAdjustedSignExp);
    f4Exp = b.create<arith::ShLIOp>(f4Exp, cF4MantissaWidth);
    f4Bits = b.create<arith::AddIOp>(f4Bits, f4Exp);

    // Step 3: Set mantissa to first bit.
    Value cF32FirstBitMask = createConst(loc, i32Ty, 0x400000, rewriter);
    Value man1Bit = b.create<arith::AndIOp>(f32Bits, cF32FirstBitMask);
    man1Bit = b.create<arith::ShRUIOp>(man1Bit, c0x00000016);
    Value f4Man = b.create<arith::TruncIOp>(i4Ty, man1Bit);
    f4Bits = b.create<arith::AddIOp>(f4Bits, f4Man);

    // Step 4: Special consideration for conversion to 0.5.
    Value cF32MantissaMask = createConst(loc, i32Ty, 0x7fffff, rewriter);
    Value f8Exp = b.create<arith::TruncIOp>(i8Ty, biasAdjustedSignExp);
    Value isSubnormal =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sle, f8Exp, c0x00);
    Value isNegOneExp =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::eq, f8Exp, c0xff);
    Value man23Bits = b.create<arith::AndIOp>(f32Bits, cF32MantissaMask);
    Value isNonZeroMan = b.create<arith::CmpIOp>(arith::CmpIPredicate::ugt,
                                                 man23Bits, zeroExpBits);
    Value roundToHalf = b.create<arith::AndIOp>(isNegOneExp, isNonZeroMan);
    Value isZeroExp =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::eq, f8Exp, c0x00);
    Value subnormalF4Bits = createConst(loc, i4Ty, 0xf, rewriter);
    Value halfF4Bits = createConst(loc, i4Ty, 0x0, rewriter);
    Value subResult =
        b.create<arith::SelectOp>(isSubnormal, subnormalF4Bits, f4Bits);
    subResult = b.create<arith::SelectOp>(roundToHalf, halfF4Bits, subResult);
    f4Bits = b.create<arith::SelectOp>(isZeroExp, f4Bits, subResult);

    // Step 5: Round up if necessary.
    Value cF32Last22BitMask = createConst(loc, i32Ty, 0x3fffff, rewriter);
    Value cRound = createConst(loc, i32Ty, 0x200000, rewriter); // 010 0000...
    Value man22Bits = b.create<arith::AndIOp>(f32Bits, cF32Last22BitMask);
    Value shouldRound =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::uge, man22Bits, cRound);
    shouldRound = b.create<arith::OrIOp>(shouldRound, isSubnormal);
    Value roundedF4Bits = b.create<arith::AddIOp>(f4Bits, c0x1);
    f4Bits = b.create<arith::SelectOp>(shouldRound, roundedF4Bits, f4Bits);

    Value result = b.create<arith::BitcastOp>(resultTy, f4Bits);
    rewriter.replaceOp(op, result);
    return success();
  }
};

struct F32ToF8E4M3FNTruncFOpConverter
    : public OpRewritePattern<arith::TruncFOp> {
  using OpRewritePattern::OpRewritePattern;
  LogicalResult matchAndRewrite(arith::TruncFOp op,
                                PatternRewriter &rewriter) const final {
    ImplicitLocOpBuilder b(op.getLoc(), rewriter);
    auto operand = op.getOperand();
    Type operandTy = operand.getType();
    Type resultTy = op.getType();
    Type operandETy = getElementTypeOrSelf(operandTy);
    Type resultETy = getElementTypeOrSelf(resultTy);

    // Match only f32 → f8E4M3
    if (!operandETy.isF32() || !llvm::isa<Float8E4M3FNType>(resultETy)) {
      return rewriter.notifyMatchFailure(op, "not a trunc of f32 to f8E4M3.");
    }

    // Integer and float shaped types matching the input shape
    Type i8Ty = b.getI8Type();
    Type i32Ty = b.getI32Type();
    if (auto shapedTy = dyn_cast<ShapedType>(operandTy)) {
      i8Ty = shapedTy.clone(i8Ty);
      i32Ty = shapedTy.clone(i32Ty);
    }

    // Bitcast f32 to raw uint32
    Value bits32 = b.create<arith::BitcastOp>(i32Ty, operand);

    // Constants
    Value bias127 = createConst(op.getLoc(), i32Ty, 127, rewriter);
    Value bias7 = createConst(op.getLoc(), i32Ty, 7, rewriter);

    // Extract sign
    Value sign = b.create<arith::ShRUIOp>(
        bits32, createConst(op.getLoc(), i32Ty, 31, rewriter));
    sign = b.create<arith::AndIOp>(
        sign, createConst(op.getLoc(), i32Ty, 0x1, rewriter));

    // Extract exponent
    Value exponent = b.create<arith::ShRUIOp>(
        bits32, createConst(op.getLoc(), i32Ty, 23, rewriter));
    exponent = b.create<arith::AndIOp>(
        exponent, createConst(op.getLoc(), i32Ty, 0xFF, rewriter));
    exponent = b.create<arith::SubIOp>(exponent, bias127);

    // Extract the mantissa
    Value mantissa = b.create<arith::AndIOp>(
        bits32, createConst(op.getLoc(), i32Ty, 0x7FFFFF, rewriter));

    // For normal numbers, add the implicit leading 1 in the mantissa
    mantissa = b.create<arith::OrIOp>(
        mantissa, createConst(op.getLoc(), i32Ty, 0x800000, rewriter));

    // Apply the bias for e4m3 (bias of 7)
    Value e4m3_exponent = b.create<arith::AddIOp>(exponent, bias7);

    // if e4m3_exponent > 15
    Value isOverflow =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sgt, e4m3_exponent,
                                createConst(op.getLoc(), i32Ty, 15, rewriter));

    // Clamp to max finite value
    Value maxFinite =
        createConst(op.getLoc(), i32Ty, 0x7F, rewriter); // 0b01111111 in f8

    // if ((e4m3_exponent > -3) && (e4m3_exponent <= 0))
    Value isSubnormal =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sge, e4m3_exponent,
                                createConst(op.getLoc(), i32Ty, -3, rewriter));
    isSubnormal = b.create<arith::AndIOp>(
        isSubnormal,
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sle, e4m3_exponent,
                                createConst(op.getLoc(), i32Ty, 0, rewriter)));

    Value shift_bits = b.create<arith::AddIOp>(
        e4m3_exponent, createConst(op.getLoc(), i32Ty, 3, rewriter));
    Value e4m3_mantissa_subnormal = b.create<arith::ShRUIOp>(
        mantissa,
        b.create<arith::SubIOp>(createConst(op.getLoc(), i32Ty, 24, rewriter),
                                shift_bits));
    e4m3_mantissa_subnormal = b.create<arith::AndIOp>(
        e4m3_mantissa_subnormal,
        b.create<arith::ShRUIOp>(
            createConst(op.getLoc(), i32Ty, 0x7, rewriter),
            b.create<arith::SubIOp>(
                createConst(op.getLoc(), i32Ty, 0, rewriter), e4m3_exponent)));

    Value resultSubnormal = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 7, rewriter)),
        b.create<arith::OrIOp>(
            createConst(op.getLoc(), i32Ty, 0x00,
                        rewriter), // Exponent is 0 for subnormals
            e4m3_mantissa_subnormal));

    // else if e4m3_exponent <= -3
    Value isZero =
        b.create<arith::CmpIOp>(arith::CmpIPredicate::sle, e4m3_exponent,
                                createConst(op.getLoc(), i32Ty, -3, rewriter));

    Value resultZero =
        createConst(op.getLoc(), i32Ty, 0x00, rewriter); // 0b00000000

    // For normal numbers, normalize mantissa to fit into 3 bits (e4m3 has 3
    // bits for mantissa)
    Value e4m3_mantissa = b.create<arith::ShRUIOp>(
        mantissa, createConst(op.getLoc(), i32Ty, 20, rewriter));
    e4m3_mantissa = b.create<arith::AndIOp>(
        e4m3_mantissa, createConst(op.getLoc(), i32Ty, 0x7, rewriter));

    // Pack the sign, exponent, and mantissa into an 8-bit value (normal
    // numbers)
    Value result = b.create<arith::OrIOp>(
        b.create<arith::ShLIOp>(sign,
                                createConst(op.getLoc(), i32Ty, 7, rewriter)),
        b.create<arith::OrIOp>(
            b.create<arith::ShLIOp>(
                e4m3_exponent, createConst(op.getLoc(), i32Ty, 3, rewriter)),
            e4m3_mantissa));

    // compute final result (if no codition is met, result is normal)
    result = b.create<arith::SelectOp>(
        isOverflow, maxFinite,
        b.create<arith::SelectOp>(
            isSubnormal, resultSubnormal,
            b.create<arith::SelectOp>(isZero, resultZero, result)));

    // Truncate to i8 and bitcast to f8e4m3
    result = b.create<arith::TruncIOp>(i8Ty, result);
    result = b.create<arith::BitcastOp>(resultTy, result);

    rewriter.replaceOp(op, result);
    return success();
  }
};

struct ArithExpandOpsPass
    : public arith::impl::ArithExpandOpsPassBase<ArithExpandOpsPass> {
  using ArithExpandOpsPassBase::ArithExpandOpsPassBase;

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    ConversionTarget target(getContext());

    arith::populateArithExpandOpsPatterns(patterns);

    target.addLegalDialect<arith::ArithDialect>();
    target.addLegalDialect<vector::VectorDialect>();

    // clang-format off
    target.addIllegalOp<
      arith::CeilDivSIOp,
      arith::CeilDivUIOp,
      arith::FloorDivSIOp,
      arith::MaxSIOp,
      arith::MaxUIOp,
      arith::MinSIOp,
      arith::MinUIOp,
      arith::MaximumFOp,
      arith::MinimumFOp,
      arith::MaxNumFOp,
      arith::MinNumFOp
    >();

    if (includeBf16)
      arith::populateExpandBFloat16Patterns(patterns);
    if (includeF8E5M2)
      arith::populateExpandF8E5M2Patterns(patterns);
    if (includeF8E4M3FN)
      arith::populateExpandF8E4M3FNPatterns(patterns);
    if (includeF4E2M1)
      arith::populateExpandF4E2M1Patterns(patterns);

    target.addDynamicallyLegalOp<arith::ExtFOp>(
      [=](arith::ExtFOp op) {
        Type inETy = getElementTypeOrSelf(op.getOperand().getType());
        Type outETy = getElementTypeOrSelf(op.getType());
        bool legalTypes = true;
        if (includeBf16)
          legalTypes &= !(inETy.isBF16() && outETy.isF32());
        if (includeF8E5M2)
          legalTypes &= !isa<Float8E5M2Type>(inETy);
        if (includeF8E4M3FN)
          legalTypes &= !isa<Float8E4M3FNType>(inETy);
        if (includeF4E2M1)
          legalTypes &= !llvm::isa<Float4E2M1FNType>(inETy);
        return legalTypes;
      });

    target.addDynamicallyLegalOp<arith::TruncFOp>(
      [=](arith::TruncFOp op)  {
        Type inETy = getElementTypeOrSelf(op.getOperand().getType());
        Type outETy = getElementTypeOrSelf(op.getType());
        bool legalTypes = true;
        if (includeBf16)
          legalTypes &= !(inETy.isF32() && outETy.isBF16());
        if (includeF8E5M2)
          legalTypes &= !isa<Float8E5M2Type>(outETy);
        if (includeF8E4M3FN)
          legalTypes &= !isa<Float8E4M3FNType>(outETy);
        if (includeF4E2M1)
          legalTypes &= !llvm::isa<Float4E2M1FNType>(outETy);
        return legalTypes;
      });

    // clang-format on
    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns))))
      signalPassFailure();
  }
};

} // namespace

void mlir::arith::populateCeilFloorDivExpandOpsPatterns(
    RewritePatternSet &patterns) {
  patterns
      .add<CeilDivSIOpConverter, CeilDivUIOpConverter, FloorDivSIOpConverter>(
          patterns.getContext());
}

void mlir::arith::populateExpandBFloat16Patterns(RewritePatternSet &patterns) {
  patterns.add<BFloat16ExtFOpConverter, BFloat16TruncFOpConverter>(
      patterns.getContext());
}

void mlir::arith::populateExpandF8E5M2Patterns(RewritePatternSet &patterns) {
  patterns.add<F8E5M2ExtFOpConverter, F8E5M2TruncFOpConverter>(
      patterns.getContext());
}

void mlir::arith::populateExpandF8E4M3FNPatterns(RewritePatternSet &patterns) {
  patterns.add<F8E4M3FNExtFOpConverter, F32ToF8E4M3FNTruncFOpConverter>(
      patterns.getContext());
}

void mlir::arith::populateExpandF4E2M1Patterns(RewritePatternSet &patterns) {
  patterns.add<F4E2M1ExtFOpConverter, F4E2M1TruncFOpConverter>(
      patterns.getContext());
}

void mlir::arith::populateArithExpandOpsPatterns(RewritePatternSet &patterns) {
  populateCeilFloorDivExpandOpsPatterns(patterns);
  // clang-format off
  patterns.add<
    MaxMinIOpConverter<MaxSIOp, arith::CmpIPredicate::sgt>,
    MaxMinIOpConverter<MaxUIOp, arith::CmpIPredicate::ugt>,
    MaxMinIOpConverter<MinSIOp, arith::CmpIPredicate::slt>,
    MaxMinIOpConverter<MinUIOp, arith::CmpIPredicate::ult>,
    MaximumMinimumFOpConverter<MaximumFOp, arith::CmpFPredicate::UGT>,
    MaximumMinimumFOpConverter<MinimumFOp, arith::CmpFPredicate::ULT>,
    MaxNumMinNumFOpConverter<MaxNumFOp, arith::CmpFPredicate::UGT>,
    MaxNumMinNumFOpConverter<MinNumFOp, arith::CmpFPredicate::ULT>
   >(patterns.getContext());
  // clang-format on
}
