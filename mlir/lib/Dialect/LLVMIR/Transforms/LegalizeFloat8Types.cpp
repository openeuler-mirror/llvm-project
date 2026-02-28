//===- LegalizeFloat8Types.cpp - Replace f8 with i8 for LLVM export -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass replaces all Float8 types (f8E4M3FN, f8E5M2, etc.) with i8 in
// LLVM dialect operations. LLVM IR has no native Float8 type, so these must
// be lowered before mlir-translate --mlir-to-llvmir. Since Float8 and i8 have
// identical memory layout (both 8-bit), this is a safe type substitution for
// loads, stores, GEPs, vector ops, and bitcasts.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/LLVMIR/Transforms/LegalizeFloat8Types.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
namespace LLVM {
#define GEN_PASS_DEF_LLVMLEGALIZEFLOAT8TYPES
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h.inc"
} // namespace LLVM
} // namespace mlir

using namespace mlir;

/// Return true if the given type is a Float8 type.
static bool isFloat8Type(Type type) {
  return isa<Float8E5M2Type, Float8E4M3Type, Float8E4M3FNType,
             Float8E5M2FNUZType, Float8E4M3FNUZType,
             Float8E4M3B11FNUZType>(type);
}

/// Return true if the given type contains a Float8 type (including vectors).
static bool containsFloat8Type(Type type) {
  if (isFloat8Type(type))
    return true;
  if (auto vecType = dyn_cast<VectorType>(type))
    return isFloat8Type(vecType.getElementType());
  if (auto fixedVec = dyn_cast<LLVM::LLVMFixedVectorType>(type))
    return isFloat8Type(fixedVec.getElementType());
  if (auto scalableVec = dyn_cast<LLVM::LLVMScalableVectorType>(type))
    return isFloat8Type(scalableVec.getElementType());
  return false;
}

/// Replace Float8 element type with IntegerType(8). Preserves vector shape.
static Type legalizeType(Type type, MLIRContext *ctx) {
  auto i8Ty = IntegerType::get(ctx, 8);

  if (isFloat8Type(type))
    return i8Ty;

  if (auto vecType = dyn_cast<VectorType>(type)) {
    if (isFloat8Type(vecType.getElementType()))
      return VectorType::get(vecType.getShape(), i8Ty,
                             vecType.getScalableDims());
  }
  if (auto fixedVec = dyn_cast<LLVM::LLVMFixedVectorType>(type)) {
    if (isFloat8Type(fixedVec.getElementType()))
      return LLVM::LLVMFixedVectorType::get(i8Ty, fixedVec.getNumElements());
  }
  if (auto scalableVec = dyn_cast<LLVM::LLVMScalableVectorType>(type)) {
    if (isFloat8Type(scalableVec.getElementType()))
      return LLVM::LLVMScalableVectorType::get(i8Ty,
                                              scalableVec.getMinNumElements());
  }

  return type;
}

namespace {

struct LegalizeFloat8TypesPass
    : public LLVM::impl::LLVMLegalizeFloat8TypesBase<
          LegalizeFloat8TypesPass> {
  void runOnOperation() override {
    Operation *op = getOperation();
    MLIRContext *ctx = &getContext();
    bool changed = false;

    op->walk([&](Operation *innerOp) {
      // Check if this op has any Float8 types in results, operands, or
      // attributes.
      bool opHasFloat8 = false;
      for (Type t : innerOp->getResultTypes()) {
        if (containsFloat8Type(t)) {
          opHasFloat8 = true;
          break;
        }
      }
      if (!opHasFloat8) {
        for (Value operand : innerOp->getOperands()) {
          if (containsFloat8Type(operand.getType())) {
            opHasFloat8 = true;
            break;
          }
        }
      }
      if (!opHasFloat8) {
        if (auto gepOp = dyn_cast<LLVM::GEPOp>(innerOp)) {
          if (gepOp.getElemType() && containsFloat8Type(gepOp.getElemType()))
            opHasFloat8 = true;
        }
      }

      if (!opHasFloat8)
        return;

      // Only legalize known-safe ops where f8→i8 is a valid type
      // substitution (memory ops and vector packaging ops). These ops treat
      // f8 as an opaque 8-bit bag of bits — no arithmetic semantics.
      // If f8 appears on any other op, it means the arithmetic emulation
      // pipeline failed to promote it to f32 and blindly rewriting to i8
      // would be semantically wrong.
      bool isSafeOp = isa<LLVM::LoadOp, LLVM::GEPOp, LLVM::UndefOp,
                          LLVM::InsertElementOp, LLVM::ShuffleVectorOp,
                          LLVM::BitcastOp>(innerOp);
      if (!isSafeOp) {
        innerOp->emitWarning()
            << "LegalizeFloat8Types: unexpected Float8 type on op '"
            << innerOp->getName()
            << "'; f8 arithmetic should have been emulated to f32 earlier "
               "in the pipeline";
        return;
      }

      changed = true;

      // Handle GEP specially — need to update the elem_type attribute.
      if (auto gepOp = dyn_cast<LLVM::GEPOp>(innerOp)) {
        Type elemType = gepOp.getElemType();
        if (elemType && containsFloat8Type(elemType)) {
          Type newElemType = legalizeType(elemType, ctx);
          gepOp.setElemType(newElemType);
        }
        return;
      }

      // Handle bitcast: if source and dest are both the same after
      // legalization, the bitcast becomes a no-op identity. We can replace
      // the result with the input directly.
      if (auto bitcastOp = dyn_cast<LLVM::BitcastOp>(innerOp)) {
        Type srcType = legalizeType(bitcastOp.getArg().getType(), ctx);
        Type dstType = legalizeType(bitcastOp.getResult().getType(), ctx);
        if (srcType == dstType) {
          bitcastOp.getResult().replaceAllUsesWith(bitcastOp.getArg());
          bitcastOp->erase();
          return;
        }
        // Non-identity: update source/dest types.
        bitcastOp.getResult().setType(dstType);
        return;
      }

      // For all other safe ops: update result types in-place.
      for (Value result : innerOp->getResults()) {
        Type t = result.getType();
        if (containsFloat8Type(t)) {
          result.setType(legalizeType(t, ctx));
        }
      }
    });

    if (!changed)
      markAllAnalysesPreserved();
  }
};

} // namespace

std::unique_ptr<Pass> LLVM::createLegalizeFloat8TypesPass() {
  return std::make_unique<LegalizeFloat8TypesPass>();
}
