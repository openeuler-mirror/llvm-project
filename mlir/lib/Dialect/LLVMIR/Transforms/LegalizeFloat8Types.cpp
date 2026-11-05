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
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"

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

/// Return true if the given type contains Float8, including vectors and arrays.
static bool containsFloat8Type(Type type) {
  if (isFloat8Type(type))
    return true;
  if (auto vecType = dyn_cast<VectorType>(type))
    return isFloat8Type(vecType.getElementType());
  if (auto fixedVec = dyn_cast<LLVM::LLVMFixedVectorType>(type))
    return isFloat8Type(fixedVec.getElementType());
  if (auto scalableVec = dyn_cast<LLVM::LLVMScalableVectorType>(type))
    return isFloat8Type(scalableVec.getElementType());
  if (auto arrayType = dyn_cast<LLVM::LLVMArrayType>(type))
    return containsFloat8Type(arrayType.getElementType());
  return false;
}

/// Replace Float8 with i8, preserving vector shape and array nesting.
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

  if (auto arrayType = dyn_cast<LLVM::LLVMArrayType>(type)) {
    Type elementType = legalizeType(arrayType.getElementType(), ctx);
    if (elementType != arrayType.getElementType())
      return LLVM::LLVMArrayType::get(elementType, arrayType.getNumElements());
  }

  return type;
}

// Detect storage attributes as well as SSA types before rewriting any values.
static bool hasFloat8StorageType(Operation *op) {
  for (Type type : op->getResultTypes())
    if (containsFloat8Type(type))
      return true;
  for (Value operand : op->getOperands())
    if (containsFloat8Type(operand.getType()))
      return true;
  if (auto gepOp = dyn_cast<LLVM::GEPOp>(op))
    return containsFloat8Type(gepOp.getElemType());
  if (auto allocaOp = dyn_cast<LLVM::AllocaOp>(op))
    return containsFloat8Type(allocaOp.getElemType());
  return false;
}

// Stores follow SSA value types and do not require an explicit rewrite.
static bool isFloat8StoreOp(Operation *op) {
  return isa<LLVM::StoreOp, LLVM::MaskedStoreOp, LLVM::masked_scatter,
             LLVM::masked_compressstore>(op);
}

static bool isSafeFloat8StorageOp(Operation *op) {
  if (auto constantOp = dyn_cast<LLVM::ConstantOp>(op)) {
    auto value = dyn_cast<DenseElementsAttr>(constantOp.getValue());
    return value && isFloat8Type(value.getElementType());
  }

  // Struct members are not recursively legalized by this pass.
  Type containerType;
  if (auto extractOp = dyn_cast<LLVM::ExtractValueOp>(op))
    containerType = extractOp.getContainer().getType();
  if (auto insertOp = dyn_cast<LLVM::InsertValueOp>(op))
    containerType = insertOp.getContainer().getType();
  if (containerType) {
    while (auto arrayType = dyn_cast<LLVM::LLVMArrayType>(containerType))
      containerType = arrayType.getElementType();
    if (isa<LLVM::LLVMStructType>(containerType))
      return false;
  }

  // These vector and aggregate results must agree with their operands in MLIR.
  // Masked-load passthrough types are handled independently by the exporter.
  if (isa<LLVM::InsertElementOp, LLVM::ExtractValueOp, LLVM::InsertValueOp,
          LLVM::ShuffleVectorOp, LLVM::BitcastOp>(op)) {
    // Block arguments and function signatures are not converted by this pass.
    // Preserve operations whose Float8 operands depend on such arguments.
    SmallVector<Value> worklist(op->operand_begin(), op->operand_end());
    llvm::SmallDenseSet<Value, 8> visited;
    while (!worklist.empty()) {
      Value value = worklist.pop_back_val();
      if (!visited.insert(value).second)
        continue;
      Operation *producer = value.getDefiningOp();
      if (!producer) {
        if (containsFloat8Type(value.getType()))
          return false;
        continue;
      }
      // Bitcasts can hide Float8 dependencies behind integer values.
      worklist.append(producer->operand_begin(), producer->operand_end());
    }
  }

  return isa<LLVM::LoadOp, LLVM::MaskedLoadOp, LLVM::GEPOp, LLVM::AllocaOp,
             LLVM::UndefOp, LLVM::InsertElementOp, LLVM::ExtractValueOp,
             LLVM::InsertValueOp, LLVM::ShuffleVectorOp, LLVM::BitcastOp>(op);
}

namespace {

struct LegalizeFloat8TypesPass
    : public LLVM::impl::LLVMLegalizeFloat8TypesBase<
          LegalizeFloat8TypesPass> {
  void runOnOperation() override {
    Operation *op = getOperation();
    MLIRContext *ctx = &getContext();
    bool changed = false;

    auto warnUnexpectedFloat8 = [](Operation *innerOp) {
      innerOp->emitWarning()
          << "LegalizeFloat8Types: unexpected Float8 type on op '"
          << innerOp->getName()
          << "'; f8 arithmetic should have been emulated to f32 earlier "
             "in the pipeline";
    };

    // Reject unsupported users before a producer changes their operand types.
    // Stores follow their SSA value types and need no explicit rewriting.
    bool hasUnsupportedFloat8 = false;
    op->walk([&](Operation *innerOp) {
      if (hasFloat8StorageType(innerOp) &&
          !isFloat8StoreOp(innerOp) &&
          !isSafeFloat8StorageOp(innerOp)) {
        warnUnexpectedFloat8(innerOp);
        hasUnsupportedFloat8 = true;
      }
    });
    if (hasUnsupportedFloat8) {
      markAllAnalysesPreserved();
      return;
    }

    op->walk([&](Operation *innerOp) {
      if (!hasFloat8StorageType(innerOp) || isFloat8StoreOp(innerOp))
        return;

      // Preserve the Float8 encoding when changing a dense constant's type.
      if (auto constantOp = dyn_cast<LLVM::ConstantOp>(innerOp)) {
        if (auto value = dyn_cast<DenseElementsAttr>(constantOp.getValue())) {
          if (isFloat8Type(value.getElementType())) {
            constantOp.setValueAttr(value.bitcast(IntegerType::get(ctx, 8)));
            for (Value result : constantOp->getResults())
              result.setType(legalizeType(result.getType(), ctx));
            changed = true;
            return;
          }
        }
      }

      if (!isSafeFloat8StorageOp(innerOp)) {
        warnUnexpectedFloat8(innerOp);
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

      // Alloca stores its element type in an attribute, not its pointer result.
      if (auto allocaOp = dyn_cast<LLVM::AllocaOp>(innerOp)) {
        Type elemType = allocaOp.getElemType();
        if (containsFloat8Type(elemType))
          allocaOp.setElemType(legalizeType(elemType, ctx));
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
