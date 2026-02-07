//===- Hoisting.cpp - Linalg hoisting transformations ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements functions concerned with hoisting invariant operations
// in the context of Linalg transformations.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Linalg/Transforms/Hoisting.h"
#include "mlir/Analysis/SliceAnalysis.h"
#include "mlir/Dialect/Affine/Analysis/AffineStructures.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Affine/IR/AffineValueMap.h"
#include "mlir/Dialect/Affine/Utils.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Utils/Utils.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/Dialect/Vector/Utils/VectorUtils.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Dominance.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Transforms/LoopInvariantCodeMotionUtils.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/TypeSwitch.h"
#include "llvm/Support/Debug.h"

using llvm::dbgs;

#define DEBUG_TYPE "linalg-hoisting"

#define DBGS() (dbgs() << '[' << DEBUG_TYPE << "] ")

using namespace mlir;
using namespace mlir::linalg;

/// Replace `loop` with a new loop that has a different init operand at
/// position `index`. The body of this loop is moved over to the new loop.
///
/// `newInitOperands` specifies the replacement "init" operands.
/// `newYieldValue` is the replacement yield value of the loop at position
/// `index`.
static scf::ForOp replaceWithDifferentYield(RewriterBase &rewriter,
                                            scf::ForOp loop,
                                            Value newInitOperand,
                                            unsigned index,
                                            Value newYieldValue) {
  OpBuilder::InsertionGuard g(rewriter);
  rewriter.setInsertionPoint(loop.getOperation());
  auto inits = llvm::to_vector(loop.getInits());

  // Replace the init value with the new operand.
  assert(index < inits.size());
  inits[index] = newInitOperand;

  scf::ForOp newLoop = rewriter.create<scf::ForOp>(
      loop.getLoc(), loop.getLowerBound(), loop.getUpperBound(), loop.getStep(),
      inits, [](OpBuilder &, Location, Value, ValueRange) {});

  // Generate the new yield with the replaced operand.
  auto yieldOp = cast<scf::YieldOp>(loop.getBody()->getTerminator());
  yieldOp.setOperand(index, newYieldValue);

  // Move the loop body to the new op.
  rewriter.mergeBlocks(loop.getBody(), newLoop.getBody(),
                       newLoop.getBody()->getArguments());

  // Replace the old loop.
  rewriter.replaceOp(loop.getOperation(), newLoop->getResults());
  return newLoop;
}

// Hoist out a pair of corresponding vector.extract+vector.broadcast
// operations. This function transforms a loop like this:
//  %res = scf.for _ = _ to _ step _ iter_args(%iarg = %v) -> (t1) {
//   %e = vector.extract %iarg : t1 to t2
//   %u = "some_use"(%e) : (t2) -> t2
//   %b = vector.broadcast %u : t2 to t1
//   scf.yield %b : t1
//  }
// into the following:
//  %e = vector.extract %v: t1 to t2
//  %res' = scf.for _ = _ to _ step _ iter_args(%iarg = %e) -> (t2) {
//   %u' = "some_use"(%iarg) : (t2) -> t2
//   scf.yield %u' : t2
//  }
//  %res = vector.broadcast %res' : t2 to t1
void mlir::linalg::hoistRedundantVectorBroadcasts(RewriterBase &rewriter,
                                                  Operation *root) {
  bool changed = true;
  while (changed) {
    changed = false;
    // First move loop invariant ops outside of their loop. This needs to be
    // done before as we cannot move ops without interrupting the function walk.
    root->walk(
        [&](LoopLikeOpInterface loopLike) { moveLoopInvariantCode(loopLike); });

    root->walk([&](vector::ExtractOp extractOp) {
      LLVM_DEBUG(DBGS() << "Candidate for hoisting: "
                        << *extractOp.getOperation() << "\n");

      auto loop = dyn_cast<scf::ForOp>(extractOp->getParentOp());
      if (!loop)
        return WalkResult::advance();

      // Check that the vector to extract from is a BlockArgument.
      auto blockArg = dyn_cast<BlockArgument>(extractOp.getVector());
      if (!blockArg)
        return WalkResult::advance();

      // Check that the blockArg is an iter_arg of the loop.
      OpOperand *initArg = loop.getTiedLoopInit(blockArg);
      if (!initArg)
        return WalkResult::advance();

      // If the iter_arg does not have only one use, it won't be possible to
      // hoist the extractOp out.
      if (!blockArg.hasOneUse())
        return WalkResult::advance();

      unsigned index = blockArg.getArgNumber() - loop.getNumInductionVars();

      // Check that the loop yields a broadcast that has just one use.
      Operation *yieldedVal =
          loop.getTiedLoopYieldedValue(blockArg)->get().getDefiningOp();
      auto broadcast = dyn_cast<vector::BroadcastOp>(yieldedVal);
      if (!broadcast || !broadcast.getResult().hasOneUse())
        return WalkResult::advance();

      LLVM_DEBUG(DBGS() << "Candidate broadcast: " << broadcast << "\n");

      Type broadcastInputType = broadcast.getSourceType();
      if (broadcastInputType != extractOp.getType())
        return WalkResult::advance();

      // The position of the extract must be defined outside of the loop if
      // it is dynamic.
      for (auto operand : extractOp.getDynamicPosition())
        if (!loop.isDefinedOutsideOfLoop(operand))
          return WalkResult::advance();

      rewriter.modifyOpInPlace(broadcast, [&] {
        extractOp.getVectorMutable().assign(initArg->get());
      });
      loop.moveOutOfLoop(extractOp);
      rewriter.moveOpAfter(broadcast, loop);

      scf::ForOp newLoop = replaceWithDifferentYield(
          rewriter, loop, extractOp.getResult(), index, broadcast.getSource());

      LLVM_DEBUG(DBGS() << "New loop: " << newLoop << "\n");

      rewriter.replaceAllUsesWith(newLoop.getResult(index), broadcast);
      rewriter.modifyOpInPlace(
          broadcast, [&] { broadcast.setOperand(newLoop.getResult(index)); });

      changed = true;
      return WalkResult::interrupt();
    });
  }
}

static bool noAliasingUseInLoop(vector::TransferReadOp transferRead,
                                LoopLikeOpInterface loop) {
  Value source = transferRead.getSource();

  // Skip view-like Ops and retrive the actual soruce Operation
  while (auto srcOp =
             dyn_cast_or_null<ViewLikeOpInterface>(source.getDefiningOp()))
    source = srcOp.getViewSource();

  llvm::SmallVector<Operation *, 32> users(source.getUsers().begin(),
                                           source.getUsers().end());
  llvm::SmallDenseSet<Operation *, 32> processed;
  while (!users.empty()) {
    Operation *user = users.pop_back_val();
    // If the user has already been processed skip.
    if (!processed.insert(user).second)
      continue;
    if (auto viewLike = dyn_cast<ViewLikeOpInterface>(user)) {
      users.append(viewLike->getUsers().begin(), viewLike->getUsers().end());
      continue;
    }
    if (isMemoryEffectFree(user) || isa<vector::TransferReadOp>(user))
      continue;
    if (!loop->isAncestor(user))
      continue;
    return false;
  }
  return true;
}

// Hoist out vector.shape_cast operations whose operands are iter_args of a
// loop. This function transforms something like this:
//  %loop = scf.for _ = _ to _ step _ iter_args(%iterarg = %v) -> (t1) {
//   %c = vector.shape_cast %iterarg : t1 to t2
//   (... do something with %c ...)
//   scf.yield %something : t1
// }
// into the following:
//  %c = vector.shape_cast %v: t1 to t2
//  %loop = scf.for _ = _ to _ step _ iter_args(%iterarg = %c) -> (t2) {
//  (... do something with %iterarg ...)
//   %something_cast = vector.shape_cast %something : t1 to t2
//   scf.yield %something_cast : t2
// }
// %loop_cast = vector.shape_cast %loop : t1 to t2
// This transformation is most useful when the generated casts can be
// canonicalized away.
void mlir::linalg::hoistRedundantVectorCasts(Operation *root) {
  bool changed = true;
  while (changed) {
    changed = false;
    // First move loop invariant ops outside of their loop. This needs to be
    // done before as we cannot move ops without interrupting the function walk.
    root->walk(
        [&](LoopLikeOpInterface loopLike) { moveLoopInvariantCode(loopLike); });

    root->walk([&](scf::ForOp loop) {
      auto iterArgs = loop.getRegionIterArgs();
      for (auto iArg : iterArgs) {
        // If the iter_arg does not have only one use, it won't be possible to
        // hoist it out.
        if (!iArg.hasOneUse()) {
          return WalkResult::advance();
        }
        auto user = *iArg.getUsers().begin();
        // We can hoist out the iter arg if and only if its user is a shape_cast
        // operation.
        if (auto shapeCast = dyn_cast<vector::ShapeCastOp>(*user)) {
          // If all the dependences of the cast are casts themselves, then we
          // cannot hoist without risking to create an infinite loop of
          // hoistings. Give up.
          auto all_casts =
              llvm::all_of(shapeCast.getResult().getUsers(), [&](Operation *v) {
                return isa<vector::ShapeCastOp>(*v);
              });
          if (all_casts)
            return WalkResult::advance();

          Value initArg = loop.getTiedLoopInit(iArg)->get();
          OpBuilder b(shapeCast);

          // Create a new vector.shape_cast operation outside of the loop
          auto newShapeCast = b.create<vector::ShapeCastOp>(
              shapeCast.getLoc(), shapeCast.getType(), initArg);
          loop.moveOutOfLoop(newShapeCast);
          b.setInsertionPoint(loop);

          auto index = iArg.getArgNumber() - loop.getNumInductionVars();
          auto operands = llvm::to_vector(loop.getInits());

          // Create new loop with the hoisted operation as an iter_arg
          operands[index] = newShapeCast.getResult();
          scf::ForOp newLoop = b.create<scf::ForOp>(
              loop.getLoc(), loop.getLowerBound(), loop.getUpperBound(),
              loop.getStep(), operands,
              [](OpBuilder &, Location, Value, ValueRange) {});

          Block *loopBody = loop.getBody();
          Block *newLoopBody = newLoop.getBody();

          // Move the body of the original loop to the new loop.
          newLoopBody->getOperations().splice(newLoopBody->end(),
                                              loopBody->getOperations());

          auto yield = cast<scf::YieldOp>(newLoopBody->getTerminator());
          // Inject a cast before the loop yield to return values of the same
          // type as the iter_args
          b.setInsertionPoint(yield);
          auto yieldCast = b.create<vector::ShapeCastOp>(
              yield.getLoc(), shapeCast.getType(), yield.getOperand(index));
          SmallVector<Value> res(yield.getResults());
          res[index] = yieldCast;
          yield.getResultsMutable().assign(res);

          // Remap the BlockArguments from the original loop to the new loop
          // BlockArguments.
          MutableArrayRef<BlockArgument> bbArgs = loopBody->getArguments();
          for (auto it :
               llvm::zip(bbArgs,
                         newLoopBody->getArguments().take_front(bbArgs.size())))
            std::get<0>(it).replaceAllUsesWith(std::get<1>(it));

          // Replace all uses of the original loop with corresponding values
          // from the new loop, and insert a cast when the result corresponds to
          // the hoisted iter_arg.
          b.setInsertionPointAfter(newLoop);
          auto castOp = b.create<vector::ShapeCastOp>(
              newLoop.getLoc(), newShapeCast.getSource().getType(),
              newLoop.getResult(index));
          auto resultToCast = loop.getResult(index);
          for (auto res : llvm::zip(loop.getResults(), newLoop.getResults())) {
            std::get<0>(res).replaceUsesWithIf(
                std::get<1>(res),
                [&](OpOperand &v) { return v.get() != resultToCast; });
          }
          resultToCast.replaceAllUsesWith(castOp);

          // Erase the old loop
          loop.erase();

          // Update the uses of the hoisted operation and erase it
          shapeCast.replaceAllUsesWith(newLoop.getRegionIterArgs()[index]);
          shapeCast.erase();

          changed = true;

          // Need to interrupt and restart because erasing the loop messes up
          // the walk.
          return WalkResult::interrupt();
        }
      }
      return WalkResult::advance();
    });
  }
}



void mlir::linalg::hoistRedundantVectorTransfers(Operation *root) {
  bool changed = true;
  while (changed) {
    changed = false;
    // First move loop invariant ops outside of their loop. This needs to be
    // done before as we cannot move ops without interrupting the function walk.
    root->walk(
        [&](LoopLikeOpInterface loopLike) { moveLoopInvariantCode(loopLike); });

    root->walk([&](vector::TransferReadOp transferRead) {
      if (!isa<MemRefType>(transferRead.getShapedType()))
        return WalkResult::advance();

      LLVM_DEBUG(DBGS() << "Candidate for hoisting: "
                        << *transferRead.getOperation() << "\n");
      auto loop = dyn_cast<LoopLikeOpInterface>(transferRead->getParentOp());
      LLVM_DEBUG(DBGS() << "Parent op: " << *transferRead->getParentOp()
                        << "\n");
      if (!isa_and_nonnull<scf::ForOp, affine::AffineForOp>(loop))
        return WalkResult::advance();

      LLVM_DEBUG(DBGS() << "Candidate read: " << *transferRead.getOperation()
                        << "\n");

      SetVector<Operation *> forwardSlice;
      getForwardSlice(transferRead.getOperation(), &forwardSlice);

      // Look for the last TransferWriteOp in the forwardSlice of
      // `transferRead` that operates on the same memref.
      vector::TransferWriteOp transferWrite;
      for (auto *sliceOp : llvm::reverse(forwardSlice)) {
        auto candidateWrite = dyn_cast<vector::TransferWriteOp>(sliceOp);
        if (!candidateWrite ||
            candidateWrite.getSource() != transferRead.getSource())
          continue;
        transferWrite = candidateWrite;
      }

      // All operands of the TransferRead must be defined outside of the loop.
      for (auto operand : transferRead.getOperands())
        if (!loop.isDefinedOutsideOfLoop(operand))
          return WalkResult::advance();

      // Only hoist transfer_read / transfer_write pairs and singleton
      // transfer_reads for now.
      if (!transferWrite) {
        // Make sure there are no other accesses to the memref before
        // hoisting transfer_read.
        if (noAliasingUseInLoop(transferRead, loop))
          loop.moveOutOfLoop(transferRead);
        return WalkResult::advance();
      }

      LLVM_DEBUG(DBGS() << "Candidate: " << *transferWrite.getOperation()
                        << "\n");

      // Approximate aliasing by checking that:
      //   1. indices, vector type and permutation map are the same (i.e., the
      //      transfer_read/transfer_write ops are matching),
      //   2. source operands for transfer.{read|write} do not originate from
      //      Ops implementing ViewLikeOpInterface.
      //   3. no other operations in the loop access the same memref except
      //      for transfer_read/transfer_write accessing statically disjoint
      //      slices.
      if (transferRead.getIndices() != transferWrite.getIndices() ||
          transferRead.getVectorType() != transferWrite.getVectorType() ||
          transferRead.getPermutationMap() != transferWrite.getPermutationMap())
        return WalkResult::advance();

      auto *source = transferRead.getSource().getDefiningOp();
      if (source && isa_and_nonnull<ViewLikeOpInterface>(source))
        return WalkResult::advance();

      source = transferWrite.getSource().getDefiningOp();
      if (source && isa_and_nonnull<ViewLikeOpInterface>(source))
        return WalkResult::advance();

      // TODO: may want to memoize this information for performance but it
      // likely gets invalidated often.
      DominanceInfo dom(loop);
      if (!dom.properlyDominates(transferRead.getOperation(), transferWrite))
        return WalkResult::advance();
      for (auto &use : transferRead.getSource().getUses()) {
        if (!loop->isAncestor(use.getOwner()))
          continue;
        if (use.getOwner() == transferRead.getOperation() ||
            use.getOwner() == transferWrite.getOperation())
          continue;
        if (auto transferWriteUse =
                dyn_cast<vector::TransferWriteOp>(use.getOwner())) {
          if (!vector::isDisjointTransferSet(
                  cast<VectorTransferOpInterface>(*transferWrite),
                  cast<VectorTransferOpInterface>(*transferWriteUse),
                  /*testDynamicValueUsingBounds=*/true))
            return WalkResult::advance();
        } else if (auto transferReadUse =
                       dyn_cast<vector::TransferReadOp>(use.getOwner())) {
          if (!vector::isDisjointTransferSet(
                  cast<VectorTransferOpInterface>(*transferWrite),
                  cast<VectorTransferOpInterface>(*transferReadUse),
                  /*testDynamicValueUsingBounds=*/true))
            return WalkResult::advance();
        } else {
          // Unknown use, we cannot prove that it doesn't alias with the
          // transferRead/transferWrite operations.
          return WalkResult::advance();
        }
      }

      // Hoist read before.
      loop.moveOutOfLoop(transferRead);

      // Hoist write after.
      transferWrite->moveAfter(loop);

      // Rewrite `loop` with new yields by cloning and erase the original loop.
      IRRewriter rewriter(transferRead.getContext());
      NewYieldValuesFn yieldFn = [&](OpBuilder &b, Location loc,
                                     ArrayRef<BlockArgument> newBBArgs) {
        return SmallVector<Value>{transferWrite.getVector()};
      };

      auto maybeNewLoop = loop.replaceWithAdditionalYields(
          rewriter, transferRead.getVector(),
          /*replaceInitOperandUsesInLoop=*/true, yieldFn);
      if (failed(maybeNewLoop))
        return WalkResult::interrupt();

      transferWrite.getVectorMutable().assign(
          maybeNewLoop->getOperation()->getResults().back());
      changed = true;
      // Need to interrupt and restart because erasing the loop messes up
      // the walk.
      return WalkResult::interrupt();
    });
  }
}
