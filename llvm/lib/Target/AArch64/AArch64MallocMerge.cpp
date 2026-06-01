//===- AArch64MallocMerge.cpp - Merge malloc/free groups -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements a function pass that merges malloc/free pairs into a
// single allocation.
//
// The pass works in three steps:
//
// 1. Collect malloc/free pairs.
//    a. Require a unique free.
//    b. Reject escaping allocations.
//    c. Check that the malloc and free mutually dominate and post-dominate
//       one another.
//    d. The alignment value of the points must be <= 16.
//
// 2. Group compatible pairs.
//    a. Find a common dominating malloc anchor.
//    b. Find a common post-dominating free anchor.
//    c. Require every member free to post-dominate the chosen leader malloc.
//
// 3. Rewrite the group.
//    a. Build the merged layout in decreasing alignment order.
//    b. Rewrite each original allocation to a disjoint byte range within the
//       merged allocation.
//    c. Insert the merged malloc/free pair and erase the original pairs.
//
//===----------------------------------------------------------------------===//

#include "AArch64.h"
#include "AArch64Subtarget.h"
#include "AArch64TargetMachine.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/CaptureTracking.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/MathExtras.h"
#include <cstdint>
#include <limits>

using namespace llvm;

#define DEBUG_TYPE "malloc-merge"

static cl::opt<bool> EnableMallocMerge("enable-malloc-merge", cl::Hidden,
                                       cl::desc("Enable the malloc merge pass"),
                                       cl::init(false));

STATISTIC(NumMergedMallocGroups, "Number of malloc groups merged");
STATISTIC(NumMergedMallocCalls, "Number of malloc calls removed");

namespace {

class AArch64MallocMergeImpl {
  struct MallocInfo {
    CallInst *Malloc = nullptr;
    CallInst *Free = nullptr;
    uint64_t Size = 0;
    Align RequiredAlign = Align(1);
    unsigned Order = 0;
    bool StoredPointerForwarded = false;
    SmallPtrSet<Instruction *, 8> Users;
  };

  struct MergeGroup {
    SmallVector<const MallocInfo *, 4> Infos;
    CallInst *LeaderMalloc = nullptr;
    CallInst *LastFree = nullptr;
  };

  static const Align MaxSupportedUseAlign;

public:
  AArch64MallocMergeImpl(const AArch64TargetMachine &TM, DominatorTree &DT,
                         PostDominatorTree &PDT, const DataLayout &DL)
      : TM(TM), DT(DT), PDT(PDT), DL(DL) {}

  bool run(Function &F) {
    if (!EnableMallocMerge || F.isDeclaration())
      return false;

    if (!TM.getSubtargetImpl(F) || !TM.getSubtargetImpl(F)->isHiSiliconProc())
      return false;

    SmallVector<MallocInfo, 8> MallocWorklist = collectMalloc(F);
    SmallVector<MergeGroup, 4> MergeWorklist =
        collectMergeGroup(MallocWorklist);

    bool Changed = false;
    for (MergeGroup &Group : MergeWorklist)
      Changed |= transformMergeGroup(Group);
    return Changed;
  }

private:
  const AArch64TargetMachine &TM;
  DominatorTree &DT;
  PostDominatorTree &PDT;
  const DataLayout &DL;

  static bool isMallocCall(const CallInst &CI) {
    const Function *Callee = CI.getCalledFunction();
    return Callee && Callee->getName() == "malloc" && CI.arg_size() == 1 &&
           CI.getType()->isPointerTy();
  }

  static bool isFreeCall(const CallInst &CI) {
    const Function *Callee = CI.getCalledFunction();
    return Callee && Callee->getName() == "free" && CI.arg_size() == 1;
  }

  static bool isSupportedPointerProducer(const Instruction &I) {
    return isa<BitCastInst>(I) || isa<GetElementPtrInst>(I);
  }

  static bool isFreeCompatibleValue(const Value &V, const Value &Root) {
    return &V == &Root || isa<BitCastInst>(V) || isa<LoadInst>(V);
  }

  bool isLocalForwardingSlot(Value *Slot) const {
    Value *Base = Slot->stripPointerCasts();
    while (auto *GEP = dyn_cast<GetElementPtrInst>(Base))
      Base = GEP->getPointerOperand()->stripPointerCasts();

    if (isa<AllocaInst>(Base))
      return true;

    auto *CI = dyn_cast<CallInst>(Base);
    return CI && isMallocCall(*CI);
  }

  bool collectStoredPointerUses(StoreInst &Store, CallInst &Root,
                                MallocInfo &Info,
                                SmallPtrSetImpl<Value *> &Visited) const {
    Value *Slot = Store.getPointerOperand();
    if (!Store.getValueOperand()->getType()->isPointerTy())
      return false;
    if (!isLocalForwardingSlot(Slot))
      return false;

    Info.StoredPointerForwarded = true;
    Info.RequiredAlign = std::max(Info.RequiredAlign, Store.getAlign());
    Info.Users.insert(&Store);

    for (User *U : Slot->users()) {
      auto *I = dyn_cast<Instruction>(U);
      if (!I)
        return false;
      if (I->isDebugOrPseudoInst() || I == &Store)
        continue;

      if (auto *LI = dyn_cast<LoadInst>(I)) {
        if (LI->isVolatile() || LI->getPointerOperand() != Slot ||
            LI->getAlign() > MaxSupportedUseAlign)
          return false;
        Info.RequiredAlign = std::max(Info.RequiredAlign, LI->getAlign());
        Info.Users.insert(LI);
        if (!collectUses(LI, Root, Info, Visited))
          return false;
        continue;
      }

      if (auto *SI = dyn_cast<StoreInst>(I)) {
        return false;
      }

      if (auto *CI = dyn_cast<CallInst>(I)) {
        if (isFreeCall(*CI) && CI->getArgOperand(0) == Slot)
          continue;
        return false;
      }

      if (isa<CallBase>(I) || isa<PHINode>(I) || isa<ReturnInst>(I))
        return false;

      if (I->getType()->isPointerTy())
        return false;
    }

    return true;
  }

  bool collectUses(Value *V, CallInst &Root, MallocInfo &Info,
                   SmallPtrSetImpl<Value *> &Visited) const {
    if (!Visited.insert(V).second)
      return true;

    for (User *U : V->users()) {
      auto *I = dyn_cast<Instruction>(U);
      if (!I)
        return false;
      if (I->isDebugOrPseudoInst())
        continue;

      if (auto *CI = dyn_cast<CallInst>(I)) {
        if (isFreeCall(*CI) && CI->getArgOperand(0) == V &&
            isFreeCompatibleValue(*V, Root)) {
          if (Info.Free && Info.Free != CI)
            return false;
          Info.Free = CI;
          continue;
        }
        return false;
      }

      if (isa<CallBase>(I) || isa<PHINode>(I) || isa<ReturnInst>(I))
        return false;

      if (auto *LI = dyn_cast<LoadInst>(I)) {
        if (LI->isVolatile() || LI->getPointerOperand() != V ||
            LI->getAlign() > MaxSupportedUseAlign)
          return false;
        Info.RequiredAlign = std::max(Info.RequiredAlign, LI->getAlign());
        Info.Users.insert(LI);
        continue;
      }

      if (auto *SI = dyn_cast<StoreInst>(I)) {
        if (SI->isVolatile() || SI->getAlign() > MaxSupportedUseAlign)
          return false;
        if (SI->getValueOperand() == V)
          return collectStoredPointerUses(*SI, Root, Info, Visited);
        if (SI->getPointerOperand() != V)
          return false;
        Info.RequiredAlign = std::max(Info.RequiredAlign, SI->getAlign());
        Info.Users.insert(SI);
        continue;
      }

      if (!isSupportedPointerProducer(*I))
        return false;

      Info.Users.insert(I);
      if (!collectUses(I, Root, Info, Visited))
        return false;
    }

    return true;
  }

  bool analyzeMalloc(CallInst &Malloc, MallocInfo &Info) const {
    if (!isMallocCall(Malloc))
      return false;

    auto *SizeC = dyn_cast<ConstantInt>(Malloc.getArgOperand(0));
    if (!SizeC)
      return false;

    Info.Malloc = &Malloc;
    Info.Size = SizeC->getZExtValue();

    SmallPtrSet<Value *, 16> Visited;
    if (!collectUses(&Malloc, Malloc, Info, Visited)) {
      LLVM_DEBUG(dbgs() << "MallocMerge: reject uses " << Malloc << '\n');
      return false;
    }

    if (!Info.Free || Info.RequiredAlign > MaxSupportedUseAlign) {
      LLVM_DEBUG(dbgs() << "MallocMerge: reject missing info " << Malloc
                        << '\n');
      return false;
    }

    if (!Info.StoredPointerForwarded &&
        PointerMayBeCapturedBefore(&Malloc, /*ReturnCaptures=*/true,
                                   /*StoreCaptures=*/true, Info.Free, &DT)) {
      LLVM_DEBUG(dbgs() << "MallocMerge: reject captured " << Malloc << '\n');
      return false;
    }

    if (!DT.dominates(&Malloc, Info.Free) ||
        !PDT.dominates(Info.Free, &Malloc)) {
      LLVM_DEBUG(dbgs() << "MallocMerge: reject dominance " << Malloc << '\n');
      return false;
    }

    for (Instruction *User : Info.Users)
      if (!PDT.dominates(Info.Free, User)) {
        LLVM_DEBUG(dbgs() << "MallocMerge: reject user postdom " << *User
                          << '\n');
        return false;
      }

    LLVM_DEBUG(dbgs() << "MallocMerge: collected malloc/free pair " << Malloc
                      << " / " << *Info.Free << '\n');
    return true;
  }

  SmallVector<MallocInfo, 8> collectMalloc(Function &F) const {
    SmallVector<MallocInfo, 8> Infos;
    unsigned Order = 0;

    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        auto *CI = dyn_cast<CallInst>(&I);
        if (!CI)
          continue;

        MallocInfo Info;
        if (!analyzeMalloc(*CI, Info))
          continue;

        Info.Order = Order++;
        Infos.push_back(std::move(Info));
      }
    }

    return Infos;
  }

  CallInst *findPostDominatingFree(ArrayRef<const MallocInfo *> Infos) const {
    for (const MallocInfo *Candidate : Infos) {
      bool PostDominatesAll = true;
      for (const MallocInfo *Info : Infos) {
        if (!PDT.dominates(Candidate->Free, Info->Free)) {
          PostDominatesAll = false;
          break;
        }
      }
      if (PostDominatesAll)
        return Candidate->Free;
    }
    return nullptr;
  }

  CallInst *findDominatingMalloc(ArrayRef<const MallocInfo *> Infos) const {
    for (const MallocInfo *Candidate : Infos) {
      bool DominatesAll = true;
      for (const MallocInfo *Info : Infos) {
        if (Candidate->Malloc != Info->Malloc &&
            !DT.dominates(Candidate->Malloc, Info->Malloc)) {
          DominatesAll = false;
          break;
        }
      }
      if (DominatesAll)
        return Candidate->Malloc;
    }
    return nullptr;
  }

  bool addToMergeGroup(MergeGroup &Group, const MallocInfo &Info) const {
    if (Group.Infos.empty()) {
      Group.Infos.push_back(&Info);
      Group.LeaderMalloc = Info.Malloc;
      Group.LastFree = Info.Free;
      return true;
    }

    SmallVector<const MallocInfo *, 4> NewInfos(Group.Infos.begin(),
                                                Group.Infos.end());
    NewInfos.push_back(&Info);

    CallInst *LeaderMalloc = findDominatingMalloc(NewInfos);
    CallInst *LastFree = findPostDominatingFree(NewInfos);
    if (!LeaderMalloc || !LastFree)
      return false;

    for (const MallocInfo *Member : NewInfos)
      if (!PDT.dominates(Member->Free, LeaderMalloc))
        return false;

    Group.Infos = std::move(NewInfos);
    Group.LeaderMalloc = LeaderMalloc;
    Group.LastFree = LastFree;
    return true;
  }

  SmallVector<MergeGroup, 4>
  collectMergeGroup(ArrayRef<MallocInfo> Infos) const {
    SmallVector<MergeGroup, 4> Groups;
    if (Infos.size() < 2)
      return Groups;

    SmallVector<bool, 8> Used(Infos.size(), false);

    for (unsigned I = 0, E = Infos.size(); I != E; ++I) {
      if (Used[I])
        continue;

      MergeGroup Group;
      if (!addToMergeGroup(Group, Infos[I]))
        continue;

      SmallVector<bool, 8> InGroup(Infos.size(), false);
      SmallVector<unsigned, 4> Members;
      InGroup[I] = true;
      Members.push_back(I);

      bool Added = false;
      do {
        Added = false;
        for (unsigned J = 0; J != E; ++J) {
          if (Used[J] || InGroup[J])
            continue;

          MergeGroup CandidateGroup = Group;
          if (!addToMergeGroup(CandidateGroup, Infos[J]))
            continue;

          Group = std::move(CandidateGroup);
          InGroup[J] = true;
          Members.push_back(J);
          Added = true;
        }
      } while (Added);

      if (Group.Infos.size() < 2)
        continue;

      llvm::sort(Group.Infos, [](const MallocInfo *L, const MallocInfo *R) {
        return L->Order < R->Order;
      });

      for (unsigned Index : Members)
        Used[Index] = true;

      LLVM_DEBUG(dbgs() << "MallocMerge: collected merge group with "
                        << Group.Infos.size() << " mallocs\n");
      Groups.push_back(std::move(Group));
    }

    return Groups;
  }

  bool computeMergedLayout(ArrayRef<const MallocInfo *> Infos,
                           SmallVectorImpl<uint64_t> &Offsets,
                           uint64_t &TotalSize) const {
    TotalSize = 0;
    Offsets.clear();
    Offsets.reserve(Infos.size());

    for (const MallocInfo *Info : Infos) {
      TotalSize = alignTo(TotalSize, Info->RequiredAlign.value());
      Offsets.push_back(TotalSize);
      if (Info->Size > std::numeric_limits<uint64_t>::max() - TotalSize)
        return false;
      TotalSize += Info->Size;
    }
    return true;
  }

  CallInst *createMergedMalloc(CallInst *InsertBefore, Function *MallocF,
                               IntegerType *SizeTy, CallingConv::ID MallocCC,
                               uint64_t TotalSize) const {
    IRBuilder<> Builder(InsertBefore);
    auto *MergedMalloc = Builder.CreateCall(
        MallocF, {ConstantInt::get(SizeTy, TotalSize)}, "merged.malloc");
    MergedMalloc->setCallingConv(MallocCC);
    MergedMalloc->setDebugLoc(InsertBefore->getDebugLoc());
    return MergedMalloc;
  }

  void replaceMallocUses(ArrayRef<const MallocInfo *> Infos,
                         ArrayRef<uint64_t> Offsets, CallInst *MergedMalloc,
                         IntegerType *SizeTy, Type *I8Ty) const {
    IRBuilder<> Builder(MergedMalloc->getNextNode());
    for (size_t I = 0, E = Infos.size(); I != E; ++I) {
      const MallocInfo *Info = Infos[I];
      uint64_t Offset = Offsets[I];
      Value *Replacement = MergedMalloc;
      if (Offset != 0)
        Replacement = Builder.CreateInBoundsGEP(
            I8Ty, MergedMalloc, ConstantInt::get(SizeTy, Offset),
            Info->Malloc->getName() + ".merged");
      Info->Malloc->replaceAllUsesWith(Replacement);
    }
  }

  bool transformMergeGroup(MergeGroup &Group) const {
    assert(Group.Infos.size() >= 2 && "expected a merge group");

    SmallVector<const MallocInfo *, 4> LayoutInfos(Group.Infos.begin(),
                                                   Group.Infos.end());
    llvm::sort(LayoutInfos, [](const MallocInfo *L, const MallocInfo *R) {
      if (L->RequiredAlign != R->RequiredAlign)
        return L->RequiredAlign.value() > R->RequiredAlign.value();
      return L->Order < R->Order;
    });

    uint64_t TotalSize = 0;
    SmallVector<uint64_t, 4> Offsets;
    if (!computeMergedLayout(LayoutInfos, Offsets, TotalSize))
      return false;

    CallInst *LeaderMalloc = Group.LeaderMalloc;
    Function *MallocF = LeaderMalloc->getCalledFunction();
    Function *FreeF = Group.LastFree->getCalledFunction();
    IntegerType *SizeTy =
        cast<IntegerType>(LeaderMalloc->getArgOperand(0)->getType());

    auto *MergedMalloc =
        createMergedMalloc(LeaderMalloc, MallocF, SizeTy,
                           LeaderMalloc->getCallingConv(), TotalSize);
    Type *I8Ty = Type::getInt8Ty(LeaderMalloc->getContext());
    replaceMallocUses(LayoutInfos, Offsets, MergedMalloc, SizeTy, I8Ty);

    IRBuilder<> FreeBuilder(Group.LastFree);
    auto *MergedFree = FreeBuilder.CreateCall(FreeF, {MergedMalloc});
    MergedFree->setCallingConv(Group.LastFree->getCallingConv());
    MergedFree->setDebugLoc(Group.LastFree->getDebugLoc());

    SmallVector<Instruction *, 8> ToErase;
    ToErase.reserve(Group.Infos.size() * 2);
    for (const MallocInfo *Info : Group.Infos) {
      ToErase.push_back(Info->Malloc);
      ToErase.push_back(Info->Free);
    }

    for (Instruction *I : ToErase) {
      assert(I->use_empty() && "expected merged malloc rewrite to remove uses");
      I->eraseFromParent();
    }

    ++NumMergedMallocGroups;
    NumMergedMallocCalls += Group.Infos.size() - 1;
    return true;
  }
};

const Align AArch64MallocMergeImpl::MaxSupportedUseAlign(16);

class AArch64MallocMergeLegacyPass : public FunctionPass {
public:
  static char ID;

  AArch64MallocMergeLegacyPass() : FunctionPass(ID) {
    initializeAArch64MallocMergeLegacyPassPass(
        *PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "malloc-merge"; }

  bool runOnFunction(Function &F) override {
    if (skipFunction(F))
      return false;

    auto &TPC = getAnalysis<TargetPassConfig>();
    auto &TM = TPC.getTM<AArch64TargetMachine>();
    auto &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    auto &PDT = getAnalysis<PostDominatorTreeWrapperPass>().getPostDomTree();
    const DataLayout &DL = F.getParent()->getDataLayout();
    return AArch64MallocMergeImpl(TM, DT, PDT, DL).run(F);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetPassConfig>();
    AU.addRequired<DominatorTreeWrapperPass>();
    AU.addRequired<PostDominatorTreeWrapperPass>();
    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addPreserved<PostDominatorTreeWrapperPass>();
    AU.setPreservesCFG();
    FunctionPass::getAnalysisUsage(AU);
  }
};

class AArch64MallocMergePass : public PassInfoMixin<AArch64MallocMergePass> {
public:
  explicit AArch64MallocMergePass(const AArch64TargetMachine *TM) : TM(TM) {}

  PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
    assert(TM && "AArch64MallocMergePass requires a target machine");
    DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
    PostDominatorTree &PDT = AM.getResult<PostDominatorTreeAnalysis>(F);
    const DataLayout &DL = F.getParent()->getDataLayout();

    bool Changed = AArch64MallocMergeImpl(*TM, DT, PDT, DL).run(F);
    if (!Changed)
      return PreservedAnalyses::all();

    PreservedAnalyses PA;
    PA.preserveSet<CFGAnalyses>();
    PA.preserve<DominatorTreeAnalysis>();
    PA.preserve<PostDominatorTreeAnalysis>();
    return PA;
  }

private:
  const AArch64TargetMachine *TM = nullptr;
};

} // namespace

char AArch64MallocMergeLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(AArch64MallocMergeLegacyPass, DEBUG_TYPE, "malloc-merge",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(TargetPassConfig)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_DEPENDENCY(PostDominatorTreeWrapperPass)
INITIALIZE_PASS_END(AArch64MallocMergeLegacyPass, DEBUG_TYPE, "malloc-merge",
                    false, false)

FunctionPass *llvm::createAArch64MallocMergePass() {
  return new AArch64MallocMergeLegacyPass();
}
