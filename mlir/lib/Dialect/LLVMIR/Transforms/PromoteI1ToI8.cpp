//===- PromoteI1ToI8.cpp - Promote vector<N x i1> to vector<N x i8> ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass promotes vector<N x i1> memory operations (loads and stores) in
// the LLVM dialect to use vector<N x i8> instead.
//
// When LLVM stores a vector<N x i1>, it packs the bits so that N elements
// occupy only N/8 bytes. However, when individual i1 elements are later loaded
// via scalar GEP + load i1, each element is addressed at byte granularity (1
// byte per element). This mismatch causes wrong results because the scalar
// loads read garbage for most elements.
//
// The fix is to promote vector<N x i1> stores to vector<N x i8> (with zext
// before store) and vector<N x i1> loads to vector<N x i8> (with trunc after
// load), so that each boolean element occupies a full byte in memory.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/LLVMIR/Transforms/PromoteI1ToI8.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
namespace LLVM {
#define GEN_PASS_DEF_LLVMPROMOTEI1TOI8
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h.inc"
} // namespace LLVM
} // namespace mlir

using namespace mlir;

namespace {

struct PromoteI1ToI8Pass
    : public LLVM::impl::LLVMPromoteI1ToI8Base<PromoteI1ToI8Pass> {
  void runOnOperation() override {
    Operation *op = getOperation();
    MLIRContext *ctx = &getContext();

    // Promote vector<N x i1> stores: insert zext to vector<N x i8>.
    SmallVector<LLVM::StoreOp> i1VecStores;
    op->walk([&](LLVM::StoreOp storeOp) {
      if (auto vecType = dyn_cast<VectorType>(storeOp.getValue().getType()))
        if (vecType.getElementType().isInteger(1))
          i1VecStores.push_back(storeOp);
    });
    for (auto storeOp : i1VecStores) {
      auto vecType = cast<VectorType>(storeOp.getValue().getType());
      auto i8VecType =
          VectorType::get(vecType.getShape(), IntegerType::get(ctx, 8));
      OpBuilder builder(storeOp);
      auto zext = builder.create<LLVM::ZExtOp>(storeOp.getLoc(), i8VecType,
                                               storeOp.getValue());
      storeOp->setOperand(0, zext);
    }

    // Promote vector<N x i1> loads: replace with vector<N x i8> load + trunc.
    SmallVector<LLVM::LoadOp> i1VecLoads;
    op->walk([&](LLVM::LoadOp loadOp) {
      if (auto vecType = dyn_cast<VectorType>(loadOp.getType()))
        if (vecType.getElementType().isInteger(1))
          i1VecLoads.push_back(loadOp);
    });
    for (auto loadOp : i1VecLoads) {
      auto vecType = cast<VectorType>(loadOp.getType());
      auto i8VecType =
          VectorType::get(vecType.getShape(), IntegerType::get(ctx, 8));
      OpBuilder builder(loadOp);
      builder.setInsertionPointAfter(loadOp);
      auto i8Load = builder.create<LLVM::LoadOp>(loadOp.getLoc(), i8VecType,
                                                 loadOp.getAddr());
      if (auto align = loadOp.getAlignment())
        i8Load.setAlignment(*align);
      auto trunc =
          builder.create<LLVM::TruncOp>(loadOp.getLoc(), vecType, i8Load);
      loadOp.replaceAllUsesWith(trunc.getResult());
      loadOp->erase();
    }
  }
};

} // namespace

std::unique_ptr<Pass> LLVM::createPromoteI1ToI8Pass() {
  return std::make_unique<PromoteI1ToI8Pass>();
}
