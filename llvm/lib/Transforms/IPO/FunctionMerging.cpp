//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the general function merging optimization.
//
// It identifies similarities between functions, and If profitable, merges them
// into a single function, replacing the original ones. Functions do not need
// to be identical to be merged. In fact, there is very little restriction to
// merge two function, however, the produced merged function can be larger than
// the two original functions together. For that reason, it uses the
// TargetTransformInfo analysis to estimate the code-size costs of instructions
// in order to estimate the profitability of merging two functions.
//
// This function merging transformation has three major parts:
// 1. The input functions are linearized, representing their CFGs as sequences
//    of labels and instructions.
// 2. We apply a sequence alignment algorithm, namely, the Needleman-Wunsch
//    algorithm, to identify similar code between the two linearized functions.
// 3. We use the aligned sequences to perform code generate, producing the new
//    merged function, using an extra parameter to represent the function
//    identifier.
//
// This pass integrates the function merging transformation with an exploration
// framework. For every function, the other functions are ranked based their
// degree of similarity, which is computed from the functions' fingerprints.
// Only the top candidates are analyzed in a greedy manner and if one of them
// produces a profitable result, the merged function is taken.
//
//===----------------------------------------------------------------------===//
//
// This optimization was proposed in
//
// Function Merging by Sequence Alignment (CGO'19)
// Rodrigo C. O. Rocha, Pavlos Petoumenos, Zheng Wang, Murray Cole, Hugh Leather
//
// Effective Function Merging in the SSA Form (PLDI'20)
// Rodrigo C. O. Rocha, Pavlos Petoumenos, Zheng Wang, Murray Cole, Hugh Leather
//
// HyFM: Function Merging for Free (LCTES'21)
// Rodrigo C. O. Rocha, Pavlos Petoumenos, Zheng Wang, Murray Cole, Kim
// Hazelwood, Hugh Leather
//
// F3M: Fast Focused Function Merging (CGO'22)
// Sean Sterling, Rodrigo C. O. Rocha, Hugh Leather, Kim Hazelwood, Michael
// O'Boyle, Pavlos Petoumenos
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/IPO/FunctionMerging.h"

#include "llvm/ADT/STLFunctionalExtras.h"
#include "llvm/ADT/SequenceAlignment.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/Twine.h"
#include "llvm/ADT/iterator_range.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/InlineSizeEstimatorAnalysis.h"
#include "llvm/Analysis/InstructionSimplify.h"
#include "llvm/Analysis/IteratedDominanceFrontier.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Operator.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Alignment.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/RandomNumberGenerator.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/CodeExtractor.h"
#include "llvm/Transforms/Utils/FunctionComparator.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"
#include "llvm/Transforms/Utils/PromoteMemToReg.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <functional>
#include <iterator>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#ifdef __unix__
/* __unix__ is usually defined by compilers targeting Unix systems */
#include <unistd.h>
#elif defined(_WIN32) || defined(WIN32)
/* _Win32 is usually defined by compilers targeting 32 or   64 bit Windows
 * systems */
#include <windows.h>
#endif

#define DEBUG_TYPE "func-merging"

using namespace llvm;

// feisen
static Value *getPossibleValueFm(Type *t, BasicBlock *pred) {
  Value *v = nullptr;
  for (Instruction &I : *pred) {
    if (LandingPadInst *landingPadInst = dyn_cast<LandingPadInst>(&I)) {
      if (landingPadInst->getType() == t) {
        v = landingPadInst;
        // v = landingPadInst->getOperand(0);
      }
    }
    if (I.getType() == t) {
      v = &I;
    }
  }
  return v;
}

// feisen
static bool handlePhiNodeFm(PHINode *phiInst) {
  bool changed = false;
  for (BasicBlock *pred : phiInst->blocks()) {
    Value *v = phiInst->getIncomingValueForBlock(pred);
    if (UndefValue::classof(v)) {
      Type *t = v->getType();
      Value *possibleValue = getPossibleValueFm(t, pred);
      if (possibleValue != nullptr) {
        phiInst->setIncomingValueForBlock(pred, possibleValue);
        changed = true;
      }
    }
  }
  return changed;
}

// feisen
static PreservedAnalyses resolvePHI_fm(Module &M, ModuleAnalysisManager &AM) {
  bool changed = false;
  for (Function &F : M) {

    std::string fName = F.getName().str();
    if (fName.size() < 5) {
      continue;
    }
    if (fName.at(0) != '_' || fName.at(1) != 'm' || fName.at(2) != '_' ||
        fName.at(3) != 'f' || fName.at(4) != '_') {
      continue;
    }

    for (BasicBlock &B : F) {
      for (Instruction &I : B) {
        if (PHINode *phiInst = dyn_cast<PHINode>(&I)) {
          changed |= handlePhiNodeFm(phiInst);
        }
      }
    }
  }
  if (changed) {
    return PreservedAnalyses::none();
  }
  return PreservedAnalyses::all();
}

static bool resolvePHI(Function &F) {
  bool changed = false;
  std::string fName = F.getName().str();
  if (fName.size() < 5) {
    return false;
  }
  if (fName.at(0) != '_' || fName.at(1) != 'm' || fName.at(2) != '_' ||
      fName.at(3) != 'f' || fName.at(4) != '_') {
    return false;
  }

  for (BasicBlock &B : F) {
    for (Instruction &I : B) {
      if (PHINode *phiInst = dyn_cast<PHINode>(&I)) {
        changed |= handlePhiNodeFm(phiInst);
      }
    }
  }
  return changed;
}
//-----

static cl::opt<unsigned> ExplorationThreshold("func-merging-explore",
                                              cl::init(10), cl::Hidden);
static cl::opt<unsigned> RankingThreshold("func-merging-ranking-threshold",
                                          cl::init(100), cl::Hidden);
static cl::opt<int> MergingOverheadThreshold("func-merging-threshold",
                                             cl::init(50), cl::Hidden);
static cl::opt<bool> MaxParamScore("func-merging-max-param", cl::init(true),
                                   cl::Hidden);
static cl::opt<bool> Debug("func-merging-debug", cl::init(false), cl::Hidden);
static cl::opt<bool> Verbose("func-merging-verbose", cl::init(false),
                             cl::Hidden);
static cl::opt<bool> IdenticalType("func-merging-identic-type",
                                   cl::init(false));
static cl::opt<bool> EnableUnifiedReturnType("func-merging-unify-return",
                                             cl::init(true), cl::Hidden);
static cl::opt<bool> EnableOperandReordering(
    "func-merging-operand-reorder", cl::init(false),
    cl::Hidden); 
static cl::opt<bool> HasWholeProgram("func-merging-whole-program",
                                     cl::init(false));
static cl::opt<bool>
    EnableHyFMPA("func-merging-hyfm-pa", cl::init(false),
                 cl::Hidden); 
static cl::opt<bool> EnableHyFMNW(
    "func-merging-hyfm-nw", cl::init(true),
    cl::Hidden); 
static cl::opt<bool> EnableSALSSACoalescing("func-merging-coalescing",
                                            cl::init(true), cl::Hidden);
static cl::opt<bool> ReuseMergedFunctions("func-merging-reuse-merges",
                                          cl::init(true), cl::Hidden);
static cl::opt<unsigned> MaxNumSelection("func-merging-max-selects",
                                         cl::init(500), cl::Hidden);
static cl::opt<bool> HyFMProfitability("hyfm-profitability", cl::init(true),
                                       cl::Hidden);
static cl::opt<bool> EnableF3M("func-merging-f3m", cl::init(true), cl::Hidden);
static cl::opt<unsigned> LSHRows("hyfm-f3m-rows", cl::init(2), cl::Hidden);
static cl::opt<unsigned> LSHBands("hyfm-f3m-bands", cl::init(100), cl::Hidden);
static cl::opt<bool> ShingleCrossBBs("shingling-cross-basic-blocks",
                                     cl::init(true));
static cl::opt<bool> AdaptiveThreshold("adaptive-threshold", cl::init(true),
                                       cl::Hidden);
static cl::opt<bool> AdaptiveBands("adaptive-bands", cl::init(true),
                                   cl::Hidden);
static cl::opt<double> RankingDistance("ranking-distance", cl::init(1.0),
                                       cl::Hidden);
static cl::opt<bool> EnableThunkPrediction("thunk-predictor", cl::init(false),
                                           cl::Hidden);
static cl::opt<bool> ReportStats("func-merging-report", cl::init(false),
                                 cl::Hidden);
static cl::opt<bool> MatcherStats("func-merging-matcher-report",
                                  cl::init(false), cl::Hidden);
static cl::opt<bool> Deterministic("func-merging-deterministic", cl::init(true),
                                   cl::Hidden);
static cl::opt<unsigned> BucketSizeCap("bucket-size-cap", cl::init(1000000000),
                                       cl::Hidden);

// Command line option to specify the function to merge. This is
// mainly used for debugging.
static cl::opt<std::string> ToMergeFile(
    "func-merging-pairs-file", cl::init(""), cl::value_desc("filename"),
    cl::desc("File containing the functions and basic blocks to merge"),
    cl::Hidden);

static std::string GetValueName(const Value *V);

#ifdef __unix__ /* __unix__ is usually defined by compilers targeting Unix     \
                   systems */

unsigned long long getTotalSystemMemory() {
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);
  return pages * page_size;
}

#elif defined(_WIN32) ||                                                       \
    defined(WIN32) /* _Win32 is usually defined by compilers targeting 32 or   \
                      64 bit Windows systems */

unsigned long long getTotalSystemMemory() {
  MEMORYSTATUSEX status;
  status.dwLength = sizeof(status);
  GlobalMemoryStatusEx(&status);
  return status.ullTotalPhys;
}

#elif defined(__APPLE__)
#include <sys/sysctl.h>
unsigned long long getTotalSystemMemory() {
  int mib[2];
  mib[0] = CTL_HW;
  mib[1] = HW_MEMSIZE;

  unsigned long long physicalMemory;
  size_t len = sizeof(physicalMemory);

  if (sysctl(mib, 2, &physicalMemory, &len, NULL, 0) == 0) {
    return physicalMemory;
  } 
    return 1024 * 1024 * 1024; // 获取失败
  
}

#endif

class FunctionMerging {
public:
  bool runImpl(Module &M) {
    TargetTransformInfo TTI(M.getDataLayout());
    auto GTTI = [&](Function &F) -> TargetTransformInfo * { return &TTI; };
    return runImpl(M, GTTI);
  }
  bool runImpl(Module &M, function_ref<TargetTransformInfo *(Function &)> GTTI);
};

FunctionMergeResult mergeFunctions(Function *F1, Function *F2,
                                   const FunctionMergingOptions &Options) {
  if (F1->getParent() != F2->getParent())
    return FunctionMergeResult(F1, F2, nullptr);
  FunctionMerger Merger(F1->getParent());
  return Merger.merge(F1, F2, "", Options);
}

static bool CmpNumbers(uint64_t L, uint64_t R) { return L == R; }

// Any two pointers in the same address space are equivalent, intptr_t and
// pointers are equivalent. Otherwise, standard type equivalence rules apply.
static bool CmpTypes(Type *TyL, Type *TyR, const DataLayout *DL) {
  auto *PTyL = dyn_cast<PointerType>(TyL);
  auto *PTyR = dyn_cast<PointerType>(TyR);

  // const DataLayout &DL = FnL->getParent()->getDataLayout();
  if (PTyL && PTyL->getAddressSpace() == 0)
    TyL = DL->getIntPtrType(TyL);
  if (PTyR && PTyR->getAddressSpace() == 0)
    TyR = DL->getIntPtrType(TyR);

  if (TyL == TyR)
    return false;

  if (int Res = CmpNumbers(TyL->getTypeID(), TyR->getTypeID()))
    return Res;

  switch (TyL->getTypeID()) {
  default:
    llvm_unreachable("Unknown type!");
  case Type::IntegerTyID:
    return CmpNumbers(cast<IntegerType>(TyL)->getBitWidth(),
                      cast<IntegerType>(TyR)->getBitWidth());
  // TyL == TyR would have returned true earlier, because types are uniqued.
  case Type::VoidTyID:
  case Type::FloatTyID:
  case Type::DoubleTyID:
  case Type::X86_FP80TyID:
  case Type::FP128TyID:
  case Type::PPC_FP128TyID:
  case Type::LabelTyID:
  case Type::MetadataTyID:
  case Type::TokenTyID:
    return false;

  case Type::PointerTyID:
    assert(PTyL && PTyR && "Both types must be pointers here.");
    return CmpNumbers(PTyL->getAddressSpace(), PTyR->getAddressSpace());

  case Type::StructTyID: {
    auto *STyL = cast<StructType>(TyL);
    auto *STyR = cast<StructType>(TyR);
    if (STyL->getNumElements() != STyR->getNumElements())
      return CmpNumbers(STyL->getNumElements(), STyR->getNumElements());

    if (STyL->isPacked() != STyR->isPacked())
      return CmpNumbers(STyL->isPacked(), STyR->isPacked());

    for (unsigned i = 0, e = STyL->getNumElements(); i != e; ++i) {
      if (int Res =
              CmpTypes(STyL->getElementType(i), STyR->getElementType(i), DL))
        return Res;
    }
    return false;
  }

  case Type::FunctionTyID: {
    auto *FTyL = cast<FunctionType>(TyL);
    auto *FTyR = cast<FunctionType>(TyR);
    if (FTyL->getNumParams() != FTyR->getNumParams())
      return CmpNumbers(FTyL->getNumParams(), FTyR->getNumParams());

    if (FTyL->isVarArg() != FTyR->isVarArg())
      return CmpNumbers(FTyL->isVarArg(), FTyR->isVarArg());

    if (int Res = CmpTypes(FTyL->getReturnType(), FTyR->getReturnType(), DL))
      return Res;

    for (unsigned i = 0, e = FTyL->getNumParams(); i != e; ++i) {
      if (int Res = CmpTypes(FTyL->getParamType(i), FTyR->getParamType(i), DL))
        return Res;
    }
    return false;
  }

  case Type::ArrayTyID: {
    auto *STyL = cast<ArrayType>(TyL);
    auto *STyR = cast<ArrayType>(TyR);
    if (STyL->getNumElements() != STyR->getNumElements())
      return CmpNumbers(STyL->getNumElements(), STyR->getNumElements());
    return CmpTypes(STyL->getElementType(), STyR->getElementType(), DL);
  }
  case Type::FixedVectorTyID:
  case Type::ScalableVectorTyID: {
    auto *STyL = cast<VectorType>(TyL);
    auto *STyR = cast<VectorType>(TyR);
    if (STyL->getElementCount().isScalable() !=
        STyR->getElementCount().isScalable())
      return CmpNumbers(STyL->getElementCount().isScalable(),
                        STyR->getElementCount().isScalable());
    if (STyL->getElementCount() != STyR->getElementCount())
      return CmpNumbers(STyL->getElementCount().getKnownMinValue(),
                        STyR->getElementCount().getKnownMinValue());
    return CmpTypes(STyL->getElementType(), STyR->getElementType(), DL);
  }
  }
}

// Any two pointers in the same address space are equivalent, intptr_t and
// pointers are equivalent. Otherwise, standard type equivalence rules apply.
bool FunctionMerger::areTypesEquivalent(Type *Ty1, Type *Ty2,
                                        const DataLayout *DL,
                                        const FunctionMergingOptions &Options) {
  if (Ty1 == Ty2)
    return true;
  if (Options.IdenticalTypesOnly)
    return false;

  return CmpTypes(Ty1, Ty2, DL);
}

static bool matchIntrinsicCalls(Intrinsic::ID ID, const CallBase *CI1,
                                const CallBase *CI2) {
  Function *F = CI1->getCalledFunction();
  if (!F)
    return false;
  auto ID1 = (Intrinsic::ID)F->getIntrinsicID();

  F = CI2->getCalledFunction();
  if (!F)
    return false;
  auto ID2 = (Intrinsic::ID)F->getIntrinsicID();

  if (ID1 != ID)
    return false;
  if (ID1 != ID2)
    return false;

  switch (ID) {
  default:
    break;
  case Intrinsic::coro_id: {
    break;
  }
  case Intrinsic::ctlz: // llvm.ctlz
  case Intrinsic::cttz: // llvm.cttz
    // is_zero_undef argument of bit counting intrinsics must be a constant int
    return CI1->getArgOperand(1) == CI2->getArgOperand(1);
  case Intrinsic::experimental_constrained_fadd:
  case Intrinsic::experimental_constrained_fsub:
  case Intrinsic::experimental_constrained_fmul:
  case Intrinsic::experimental_constrained_fdiv:
  case Intrinsic::experimental_constrained_frem:
  case Intrinsic::experimental_constrained_fma:
  case Intrinsic::experimental_constrained_sqrt:
  case Intrinsic::experimental_constrained_pow:
  case Intrinsic::experimental_constrained_powi:
  case Intrinsic::experimental_constrained_sin:
  case Intrinsic::experimental_constrained_cos:
  case Intrinsic::experimental_constrained_exp:
  case Intrinsic::experimental_constrained_exp2:
  case Intrinsic::experimental_constrained_log:
  case Intrinsic::experimental_constrained_log10:
  case Intrinsic::experimental_constrained_log2:
  case Intrinsic::experimental_constrained_rint:
  case Intrinsic::experimental_constrained_nearbyint:
    break;
  case Intrinsic::dbg_declare: // llvm.dbg.declare
    break;
  case Intrinsic::dbg_value: // llvm.dbg.value
    break;
  case Intrinsic::dbg_label: // llvm.dbg.label
    break;
  case Intrinsic::memcpy:
  case Intrinsic::memmove:
  case Intrinsic::memset: {
    // isvolatile argument of memory intrinsics must be a constant int
    return CI1->getArgOperand(3) == CI2->getArgOperand(3);
  }
  case Intrinsic::memcpy_element_unordered_atomic:
  case Intrinsic::memmove_element_unordered_atomic:
  case Intrinsic::memset_element_unordered_atomic: {
    const auto *AMI1 = cast<AtomicMemIntrinsic>(CI1);
    const auto *AMI2 = cast<AtomicMemIntrinsic>(CI2);

    auto *ElementSizeCI1 =
        dyn_cast<ConstantInt>(AMI1->getRawElementSizeInBytes());

    auto *ElementSizeCI2 =
        dyn_cast<ConstantInt>(AMI2->getRawElementSizeInBytes());

    return (ElementSizeCI1 != nullptr && ElementSizeCI1 == ElementSizeCI2);
  }
  case Intrinsic::gcroot:
  case Intrinsic::gcwrite:
  case Intrinsic::gcread:
    // llvm.gcroot parameter #2 must be a constant.
    return CI1->getArgOperand(1) == CI2->getArgOperand(1);
  case Intrinsic::init_trampoline:
    break;
  case Intrinsic::prefetch:
    // arguments #2 and #3 in llvm.prefetch must be constants
    return CI1->getArgOperand(1) == CI2->getArgOperand(1) &&
           CI1->getArgOperand(2) == CI2->getArgOperand(2);
  case Intrinsic::stackprotector:

    break;
  case Intrinsic::lifetime_start:
  case Intrinsic::lifetime_end:
  case Intrinsic::invariant_start:
    // size argument of memory use markers must be a constant integer
    return CI1->getArgOperand(0) == CI2->getArgOperand(0);
  case Intrinsic::invariant_end:
    // llvm.invariant.end parameter #2 must be a constant integer
    return CI1->getArgOperand(1) == CI2->getArgOperand(1);
  case Intrinsic::localescape: {

    break;
  }
  case Intrinsic::localrecover: {

    break;
  }
  };
  return false; // TODO: change to false by default
}

static bool matchLandingPad(LandingPadInst *LP1, LandingPadInst *LP2) {
  if (LP1->getType() != LP2->getType())
    return false;
  if (LP1->isCleanup() != LP2->isCleanup())
    return false;
  if (LP1->getNumClauses() != LP2->getNumClauses())
    return false;
  for (unsigned i = 0; i < LP1->getNumClauses(); i++) {
    if (LP1->isCatch(i) != LP2->isCatch(i))
      return false;
    if (LP1->isFilter(i) != LP2->isFilter(i))
      return false;
    if (LP1->getClause(i) != LP2->getClause(i))
      return false;
  }
  return true;
}

static bool matchLoadInsts(const LoadInst *LI1, const LoadInst *LI2) {
  return LI1->isVolatile() == LI2->isVolatile() &&
         LI1->getAlign() == LI2->getAlign() &&
         LI1->getOrdering() == LI2->getOrdering();
}

static bool matchStoreInsts(const StoreInst *SI1, const StoreInst *SI2) {
  return SI1->isVolatile() == SI2->isVolatile() &&
         SI1->getAlign() == SI2->getAlign() &&
         SI1->getOrdering() == SI2->getOrdering();
}

static bool matchAllocaInsts(const AllocaInst *AI1, const AllocaInst *AI2) {

  if (AI1->getArraySize() != AI2->getArraySize() ||
      AI1->getAlign() != AI2->getAlign() ||
      AI1->getAllocatedType() != AI2->getAllocatedType())
    return false;

  if (AI1->getArraySize() != AI2->getArraySize() ||
      AI1->getAlign() != AI2->getAlign())
    return false;

  return true;
}

static bool matchGetElementPtrInsts(const GetElementPtrInst *GEP1,
                                    const GetElementPtrInst *GEP2) {
  Type *Ty1 = GEP1->getSourceElementType();
  SmallVector<Value *, 16> Idxs1(GEP1->idx_begin(), GEP1->idx_end());

  Type *Ty2 = GEP2->getSourceElementType();
  SmallVector<Value *, 16> Idxs2(GEP2->idx_begin(), GEP2->idx_end());

  if (Ty1 != Ty2)
    return false;
  if (Idxs1.size() != Idxs2.size())
    return false;

  if (Idxs1.empty())
    return true;

  for (unsigned i = 1; i < Idxs1.size(); i++) {
    Value *V1 = Idxs1[i];
    Value *V2 = Idxs2[i];

    // structs must have constant indices, therefore they must be constants and
    // must be identical when merging
    if (isa<StructType>(Ty1)) {
      if (V1 != V2)
        return false;
    }
    Ty1 = GetElementPtrInst::getTypeAtIndex(Ty1, V1);
    Ty2 = GetElementPtrInst::getTypeAtIndex(Ty2, V2);
    if (Ty1 != Ty2)
      return false;
  }
  return true;
}

static bool matchSwitchInsts(const SwitchInst *SI1, const SwitchInst *SI2) {
  if (SI1->getNumCases() == SI2->getNumCases()) {
    auto CaseIt1 = SI1->case_begin(), CaseEnd1 = SI1->case_end();
    auto CaseIt2 = SI2->case_begin(), CaseEnd2 = SI2->case_end();
    do {
      auto *Case1 = &*CaseIt1;
      auto *Case2 = &*CaseIt2;
      if (Case1 != Case2)
        return false; // TODO: could allow permutation!
      ++CaseIt1;
      ++CaseIt2;
    } while (CaseIt1 != CaseEnd1 && CaseIt2 != CaseEnd2);
    return true;
  }
  return false;
}

static bool matchCallInsts(const CallBase *CI1, const CallBase *CI2) {
  if (CI1->isInlineAsm() || CI2->isInlineAsm())
    return false;

  // if (CI1->getCalledFunction()==nullptr) return false;

  if (CI1->getCalledFunction() != CI2->getCalledFunction())
    return false;

  if (Function *F = CI1->getCalledFunction()) {
    if (auto ID = (Intrinsic::ID)F->getIntrinsicID()) {
      if (!matchIntrinsicCalls(ID, CI1, CI2))
        return false;
    }
  }

  return CI1->arg_size() == CI2->arg_size() &&
         CI1->getCallingConv() == CI2->getCallingConv() &&
         CI1->getAttributes() == CI2->getAttributes();
}

static bool matchInvokeInsts(const InvokeInst *II1, const InvokeInst *II2) {
  return matchCallInsts(II1, II2) &&
         II1->getCallingConv() == II2->getCallingConv() &&
         II1->getAttributes() == II2->getAttributes() &&
         matchLandingPad(II1->getLandingPadInst(), II2->getLandingPadInst());
}

static bool matchInsertValueInsts(const InsertValueInst *IV1,
                                  const InsertValueInst *IV2) {
  return IV1->getIndices() == IV2->getIndices();
}

static bool matchExtractValueInsts(const ExtractValueInst *EV1,
                                   const ExtractValueInst *EV2) {
  return EV1->getIndices() == EV2->getIndices();
}

static bool matchFenceInsts(const FenceInst *FI1, const FenceInst *FI2) {
  return FI1->getOrdering() == FI2->getOrdering() &&
         FI1->getSyncScopeID() == FI2->getSyncScopeID();
}

bool FunctionMerger::matchInstructions(Instruction *I1, Instruction *I2,
                                       const FunctionMergingOptions &Options) {

  if (I1->getOpcode() != I2->getOpcode())
    return false;

  if (I1->getOpcode() == Instruction::CallBr)
    return false;

  // Returns are special cases that can differ in the number of operands
  if (I1->getOpcode() == Instruction::Ret)
    return true;

  if (I1->getNumOperands() != I2->getNumOperands())
    return false;

  const DataLayout *DL =
      &I1->getParent()->getParent()->getParent()->getDataLayout();

  bool sameType = false;
  if (Options.IdenticalTypesOnly) {
    sameType = (I1->getType() == I2->getType());
    for (unsigned i = 0; i < I1->getNumOperands(); i++) {
      sameType = sameType &&
                 (I1->getOperand(i)->getType() == I2->getOperand(i)->getType());
    }
  } else {
    sameType = areTypesEquivalent(I1->getType(), I2->getType(), DL, Options);
    for (unsigned i = 0; i < I1->getNumOperands(); i++) {
      sameType = sameType &&
                 areTypesEquivalent(I1->getOperand(i)->getType(),
                                    I2->getOperand(i)->getType(), DL, Options);
    }
  }
  if (!sameType)
    return false;

  switch (I1->getOpcode()) {

    // #define MatchCaseInst(Kind, I1, I2) case Instruction::#Kind
  case Instruction::ShuffleVector: // feisen:24/03/09/
    return cast<ShuffleVectorInst>(I1)->getShuffleMask() ==
           cast<ShuffleVectorInst>(I2)->getShuffleMask();
  case Instruction::Load:
    return matchLoadInsts(dyn_cast<LoadInst>(I1), dyn_cast<LoadInst>(I2));
  case Instruction::Store:
    return matchStoreInsts(dyn_cast<StoreInst>(I1), dyn_cast<StoreInst>(I2));
  case Instruction::Alloca:
    return matchAllocaInsts(dyn_cast<AllocaInst>(I1), dyn_cast<AllocaInst>(I2));
  case Instruction::GetElementPtr:
    return matchGetElementPtrInsts(dyn_cast<GetElementPtrInst>(I1),
                                   dyn_cast<GetElementPtrInst>(I2));
  case Instruction::Switch:
    return matchSwitchInsts(dyn_cast<SwitchInst>(I1), dyn_cast<SwitchInst>(I2));
  case Instruction::Call:
    return matchCallInsts(dyn_cast<CallInst>(I1), dyn_cast<CallInst>(I2));
  case Instruction::Invoke:
    return matchInvokeInsts(dyn_cast<InvokeInst>(I1), dyn_cast<InvokeInst>(I2));
  case Instruction::InsertValue:
    return matchInsertValueInsts(dyn_cast<InsertValueInst>(I1),
                                 dyn_cast<InsertValueInst>(I2));
  case Instruction::ExtractValue:
    return matchExtractValueInsts(dyn_cast<ExtractValueInst>(I1),
                                  dyn_cast<ExtractValueInst>(I2));
  case Instruction::Fence:
    return matchFenceInsts(dyn_cast<FenceInst>(I1), dyn_cast<FenceInst>(I2));
  case Instruction::AtomicCmpXchg: {
    const AtomicCmpXchgInst *CXI = dyn_cast<AtomicCmpXchgInst>(I1);
    const AtomicCmpXchgInst *CXI2 = cast<AtomicCmpXchgInst>(I2);
    return CXI->isVolatile() == CXI2->isVolatile() &&
           CXI->isWeak() == CXI2->isWeak() &&
           CXI->getSuccessOrdering() == CXI2->getSuccessOrdering() &&
           CXI->getFailureOrdering() == CXI2->getFailureOrdering() &&
           CXI->getSyncScopeID() == CXI2->getSyncScopeID();
  }
  case Instruction::AtomicRMW: {
    const AtomicRMWInst *RMWI = dyn_cast<AtomicRMWInst>(I1);
    return RMWI->getOperation() == cast<AtomicRMWInst>(I2)->getOperation() &&
           RMWI->isVolatile() == cast<AtomicRMWInst>(I2)->isVolatile() &&
           RMWI->getOrdering() == cast<AtomicRMWInst>(I2)->getOrdering() &&
           RMWI->getSyncScopeID() == cast<AtomicRMWInst>(I2)->getSyncScopeID();
  }
  default:
    if (auto *CI = dyn_cast<CmpInst>(I1))
      return CI->getPredicate() == cast<CmpInst>(I2)->getPredicate();
    if (isa<OverflowingBinaryOperator>(I1)) {
      if (!isa<OverflowingBinaryOperator>(I2))
        return false;
      if (I1->hasNoUnsignedWrap() != I2->hasNoUnsignedWrap())
        return false;
      if (I1->hasNoSignedWrap() != I2->hasNoSignedWrap())
        return false;
    }
    if (isa<PossiblyExactOperator>(I1)) {
      if (!isa<PossiblyExactOperator>(I2))
        return false;
      if (I1->isExact() != I2->isExact())
        return false;
    }
    if (isa<FPMathOperator>(I1)) {
      if (!isa<FPMathOperator>(I2))
        return false;
      if (I1->isFast() != I2->isFast())
        return false;
      if (I1->hasAllowReassoc() != I2->hasAllowReassoc())
        return false;
      if (I1->hasNoNaNs() != I2->hasNoNaNs())
        return false;
      if (I1->hasNoInfs() != I2->hasNoInfs())
        return false;
      if (I1->hasNoSignedZeros() != I2->hasNoSignedZeros())
        return false;
      if (I1->hasAllowReciprocal() != I2->hasAllowReciprocal())
        return false;
      if (I1->hasAllowContract() != I2->hasAllowContract())
        return false;
      if (I1->hasApproxFunc() != I2->hasApproxFunc())
        return false;
    }
  }

  return true;
}

bool FunctionMerger::match(Value *V1, Value *V2) {
  if (auto *I1 = dyn_cast<Instruction>(V1))
    if (auto *I2 = dyn_cast<Instruction>(V2))
      return matchInstructions(I1, I2);

  if (auto *BB1 = dyn_cast<BasicBlock>(V1))
    if (auto *BB2 = dyn_cast<BasicBlock>(V2))
      return matchBlocks(BB1, BB2);

  return false;
}

bool FunctionMerger::matchBlocks(BasicBlock *BB1, BasicBlock *BB2) {
  if (BB1 == nullptr || BB2 == nullptr)
    return false;
  if (BB1->isLandingPad() || BB2->isLandingPad()) {
    LandingPadInst *LP1 = BB1->getLandingPadInst();
    LandingPadInst *LP2 = BB2->getLandingPadInst();
    if (LP1 == nullptr || LP2 == nullptr)
      return false;
    return matchLandingPad(LP1, LP2);
  }
  return true;
}

bool FunctionMerger::matchWholeBlocks(Value *V1, Value *V2) {
  auto *BB1 = dyn_cast<BasicBlock>(V1);
  auto *BB2 = dyn_cast<BasicBlock>(V2);
  if (BB1 == nullptr || BB2 == nullptr)
    return false;

  if (!matchBlocks(BB1, BB2))
    return false;

  auto It1 = BB1->begin();
  auto It2 = BB2->begin();

  while (isa<PHINode>(*It1) || isa<LandingPadInst>(*It1))
    It1++;
  while (isa<PHINode>(*It2) || isa<LandingPadInst>(*It2))
    It2++;

  while (It1 != BB1->end() && It2 != BB2->end()) {
    if (!matchInstructions(&*It1, &*It2))
      return false;

    It1++;
    It2++;
  }

  if (It1 != BB1->end() || It2 != BB2->end())
    return false;

  return true;
}

static unsigned
RandomLinearizationOfBlocks(BasicBlock *BB,
                            std::list<BasicBlock *> &OrederedBBs,
                            std::set<BasicBlock *> &Visited) {
  if (Visited.find(BB) != Visited.end())
    return 0;
  Visited.insert(BB);

  Instruction *TI = BB->getTerminator();

  std::vector<BasicBlock *> NextBBs;
  for (unsigned i = 0; i < TI->getNumSuccessors(); i++) {
    NextBBs.push_back(TI->getSuccessor(i));
  }
  std::random_device rd;
  std::shuffle(NextBBs.begin(), NextBBs.end(), std::mt19937(rd()));

  unsigned SumSizes = 0;
  for (BasicBlock *NextBlock : NextBBs) {
    SumSizes += RandomLinearizationOfBlocks(NextBlock, OrederedBBs, Visited);
  }

  OrederedBBs.push_front(BB);
  return SumSizes + BB->size();
}

static unsigned
RandomLinearizationOfBlocks(Function *F, std::list<BasicBlock *> &OrederedBBs) {
  std::set<BasicBlock *> Visited;
  return RandomLinearizationOfBlocks(&F->getEntryBlock(), OrederedBBs, Visited);
}

static unsigned
CanonicalLinearizationOfBlocks(BasicBlock *BB,
                               std::list<BasicBlock *> &OrederedBBs,
                               std::set<BasicBlock *> &Visited) {
  if (Visited.find(BB) != Visited.end())
    return 0;
  Visited.insert(BB);

  Instruction *TI = BB->getTerminator();

  unsigned SumSizes = 0;
  for (unsigned i = 0; i < TI->getNumSuccessors(); i++) {
    SumSizes += CanonicalLinearizationOfBlocks(TI->getSuccessor(i), OrederedBBs,
                                               Visited);
  }

  OrederedBBs.push_front(BB);
  return SumSizes + BB->size();
}

static unsigned
CanonicalLinearizationOfBlocks(Function *F,
                               std::list<BasicBlock *> &OrederedBBs) {
  std::set<BasicBlock *> Visited;
  return CanonicalLinearizationOfBlocks(&F->getEntryBlock(), OrederedBBs,
                                        Visited);
}

static void vectorizeBB(SmallVectorImpl<Value *> &Vec, BasicBlock *BB) {
  Vec.push_back(BB);
  for (Instruction &I : *BB)
    if (!isa<LandingPadInst>(&I) && !isa<PHINode>(&I))
      Vec.push_back(&I);
}

void FunctionMerger::linearize(Function *F, SmallVectorImpl<Value *> &FVec,
                               FunctionMerger::LinearizationKind LK) {
  std::list<BasicBlock *> OrderedBBs;

  unsigned FReserve = 0;
  switch (LK) {
  case LinearizationKind::LK_Random:
    FReserve = RandomLinearizationOfBlocks(F, OrderedBBs);
    break;

  default:
    FReserve = CanonicalLinearizationOfBlocks(F, OrderedBBs);
    break;
  }

  FVec.reserve(FReserve + OrderedBBs.size());
  for (BasicBlock *BB : OrderedBBs)
    vectorizeBB(FVec, BB);
}

bool FunctionMerger::validMergeTypes(Function *F1, Function *F2,
                                     const FunctionMergingOptions &Options) {
  bool EquivTypes =
      areTypesEquivalent(F1->getReturnType(), F2->getReturnType(), DL, Options);
  if (!EquivTypes && !F1->getReturnType()->isVoidTy() &&
      !F2->getReturnType()->isVoidTy()) {
    return false;
  }
  return true;
}

static bool validMergePair(Function *F1, Function *F2) {
  if (!HasWholeProgram && (F1->hasAvailableExternallyLinkage() ||
                           F2->hasAvailableExternallyLinkage()))
    return false;

  if (!HasWholeProgram &&
      (F1->hasLinkOnceLinkage() || F2->hasLinkOnceLinkage()))
    return false;

  if (F1->hasComdat() != F2->hasComdat())
    return false;
  if (F1->hasComdat() && F1->getComdat() != F2->getComdat())
    return false;

  if (F1->hasPersonalityFn() != F2->hasPersonalityFn())
    return false;
  if (F1->hasPersonalityFn()) {
    Constant *PersonalityFn1 = F1->getPersonalityFn();
    Constant *PersonalityFn2 = F2->getPersonalityFn();
    if (PersonalityFn1 != PersonalityFn2)
      return false;
  }

  return true;
}

static void MergeArguments(LLVMContext &Context, Function *F1, Function *F2,
                           AlignedCode &AlignedSeq,
                           std::map<unsigned, unsigned> &ParamMap1,
                           std::map<unsigned, unsigned> &ParamMap2,
                           std::vector<Type *> &Args,
                           const FunctionMergingOptions &Options) {

  std::vector<Argument *> ArgsList1;
  for (Argument &arg : F1->args()) {
    ArgsList1.push_back(&arg);
  }

  Args.push_back(IntegerType::get(Context, 1)); // push the function Id argument
  unsigned ArgId = 0;
  for (auto I = F1->arg_begin(), E = F1->arg_end(); I != E; I++) {
    ParamMap1[ArgId] = Args.size();
    Args.push_back((*I).getType());
    ArgId++;
  }

  auto AttrList1 = F1->getAttributes();
  auto AttrList2 = F2->getAttributes();

  // merge arguments from Function2 with Function1
  ArgId = 0;
  for (auto I = F2->arg_begin(), E = F2->arg_end(); I != E; I++) {

    std::map<unsigned, int> MatchingScore;
    // first try to find an argument with the same name/type
    // otherwise try to match by type only
    for (unsigned i = 0; i < ArgsList1.size(); i++) {
      if (ArgsList1[i]->getType() == (*I).getType()) {

        auto AttrSet1 = AttrList1.getParamAttrs(ArgsList1[i]->getArgNo());
        auto AttrSet2 = AttrList2.getParamAttrs((*I).getArgNo());
        if (AttrSet1 != AttrSet2)
          continue;

        bool hasConflict = false; // check for conflict from a previous matching
        for (auto ParamPair : ParamMap2) {
          if (ParamPair.second == ParamMap1[i]) {
            hasConflict = true;
            break;
          }
        }
        if (hasConflict)
          continue;
        MatchingScore[i] = 0;
        if (!Options.MaximizeParamScore)
          break; // if not maximize score, get the first one
      }
    }

    if (MatchingScore.size() > 0) { // maximize scores
      for (auto &Entry : AlignedSeq) {
        if (Entry.match()) {
          auto *I1 = dyn_cast<Instruction>(Entry.get(0));
          auto *I2 = dyn_cast<Instruction>(Entry.get(1));
          if (I1 != nullptr && I2 != nullptr) { // test both for sanity
            for (unsigned i = 0; i < I1->getNumOperands(); i++) {
              for (auto KV : MatchingScore) {
                if (I1->getOperand(i) == ArgsList1[KV.first]) {
                  if (i < I2->getNumOperands() && I2->getOperand(i) == &(*I)) {
                    MatchingScore[KV.first]++;
                  }
                }
              }
            }
          }
        }
      }

      int MaxScore = -1;
      unsigned MaxId = 0;

      for (auto KV : MatchingScore) {
        if (KV.second > MaxScore) {
          MaxScore = KV.second;
          MaxId = KV.first;
        }
      }

      ParamMap2[ArgId] = ParamMap1[MaxId];
    } else {
      ParamMap2[ArgId] = Args.size();
      Args.push_back((*I).getType());
    }

    ArgId++;
  }
}

static void SetFunctionAttributes(Function *F1, Function *F2,
                                  Function *MergedFunc) {
  unsigned MaxAlignment = std::max(F1->getAlignment(), F2->getAlignment());
  if (F1->getAlignment() != F2->getAlignment()) {
  }
  if (MaxAlignment)
    MergedFunc->setAlignment(Align(MaxAlignment));

  if (F1->getCallingConv() == F2->getCallingConv()) {
    MergedFunc->setCallingConv(F1->getCallingConv());
  } else {
  }

  MergedFunc->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);

  MergedFunc->setDSOLocal(true);

  if (F1->getSubprogram() == F2->getSubprogram()) {
    MergedFunc->setSubprogram(F1->getSubprogram());
  } else {
  }

  MergedFunc->setVisibility(GlobalValue::VisibilityTypes::DefaultVisibility);

  // Exception Handling requires landing pads to have the same personality
  // function
  if (F1->hasPersonalityFn() && F2->hasPersonalityFn()) {
    Constant *PersonalityFn1 = F1->getPersonalityFn();
    Constant *PersonalityFn2 = F2->getPersonalityFn();
    if (PersonalityFn1 == PersonalityFn2) {
      MergedFunc->setPersonalityFn(PersonalityFn1);
    }
  } else if (F1->hasPersonalityFn()) {

    MergedFunc->setPersonalityFn(F1->getPersonalityFn());

  } else if (F2->hasPersonalityFn()) {

    MergedFunc->setPersonalityFn(F2->getPersonalityFn());
  }

  if (F1->hasComdat() && F2->hasComdat()) {
    auto *Comdat1 = F1->getComdat();
    auto *Comdat2 = F2->getComdat();
    if (Comdat1 == Comdat2) {
      MergedFunc->setComdat(Comdat1);
    }
  } else if (F1->hasComdat()) {
    MergedFunc->setComdat(F1->getComdat());
  } else if (F2->hasComdat()) {
    MergedFunc->setComdat(F2->getComdat());
  }

  if (F1->hasSection()) {
    MergedFunc->setSection(F1->getSection());
  }
}

static Value *createCastIfNeeded(Value *V, Type *DstType, IRBuilder<> &Builder,
                                 Type *IntPtrTy,
                                 const FunctionMergingOptions &Options = {});

void FunctionMerger::CodeGenerator::destroyGeneratedCode() {
  for (Instruction *I : CreatedInsts) {
    I->dropAllReferences();
  }
  for (Instruction *I : CreatedInsts) {
    I->eraseFromParent();
  }
  for (BasicBlock *BB : CreatedBBs) {
    BB->eraseFromParent();
  }
  CreatedInsts.clear();
  CreatedBBs.clear();
}

unsigned instToInt(Instruction *I);

inst_range getInstructions(Function *F) { return instructions(F); }

iterator_range<BasicBlock::iterator> getInstructions(BasicBlock *BB) {
  return make_range(BB->begin(), BB->end());
}

template <class T> class FingerprintMH {
private:
  // The number of instructions defining a shingle. 2 or 3 is best.
  static constexpr size_t K = 2;
  static constexpr double threshold = 0.3;
  static constexpr size_t MaxOpcode = 68;
  const uint32_t _footprint;

public:
  uint64_t magnitude{0};
  std::vector<uint32_t> hash;
  std::vector<uint32_t> bandHash;

public:
  FingerprintMH() = default;

  FingerprintMH(T owner, SearchStrategy &searchStrategy)
      : _footprint(searchStrategy.itemFootprint()) {
    std::vector<uint32_t> integers;
    std::array<uint32_t, MaxOpcode> OpcodeFreq;

    for (size_t i = 0; i < MaxOpcode; i++)
      OpcodeFreq[i] = 0;

    if (ShingleCrossBBs) {
      for (Instruction &I : getInstructions(owner)) {
        integers.push_back(instToInt(&I));
        OpcodeFreq[I.getOpcode()]++;
        if (I.isTerminator())
          OpcodeFreq[0] += I.getNumSuccessors();
      }
    } else {
      for (BasicBlock &BB : *owner) {

        // Process normal instructions
        for (Instruction &I : BB) {
          integers.push_back(instToInt(&I));
          OpcodeFreq[I.getOpcode()]++;
          if (I.isTerminator())
            OpcodeFreq[0] += I.getNumSuccessors();
        }

        // Add dummy instructions between basic blocks
        for (size_t i = 0; i < K - 1; i++) {
          integers.push_back(1);
        }
      }
    }

    for (size_t i = 0; i < MaxOpcode; ++i) {
      uint64_t val = OpcodeFreq[i];
      magnitude += val * val;
    }

    searchStrategy.generateShinglesMultipleHashPipelineTurbo<K>(integers, hash);
    searchStrategy.generateBands(hash, bandHash);
  }

  uint32_t footprint() const { return _footprint; }

  float distance(const FingerprintMH &FP2) const {
    size_t nintersect = 0;
    size_t pos1 = 0;
    size_t pos2 = 0;
    size_t nHashes = hash.size();

    while (pos1 != nHashes && pos2 != nHashes) {
      if (hash[pos1] == FP2.hash[pos2]) {
        nintersect++;
        pos1++;
        pos2++;
      } else if (hash[pos1] < FP2.hash[pos2]) {
        pos1++;
      } else {
        pos2++;
      }
    }

    int nunion = 2 * nHashes - nintersect;
    return 1.f - (nintersect / (float)nunion);
  }

  float distance_under(const FingerprintMH &FP2, float best_distance) const {
    size_t mismatches = 0;
    size_t pos1 = 0;
    size_t pos2 = 0;
    size_t nHashes = hash.size();
    size_t best_nintersect = static_cast<size_t>(
        2.0 * nHashes * (1.f - best_distance) / (2.f - best_distance));
    size_t best_mismatches = 2 * (nHashes - best_nintersect);

    while (pos1 != nHashes && pos2 != nHashes) {
      if (hash[pos1] == FP2.hash[pos2]) {
        pos1++;
        pos2++;
      } else if (hash[pos1] < FP2.hash[pos2]) {
        mismatches++;
        pos1++;
      } else {
        mismatches++;
        pos2++;
      }
      if (mismatches > best_mismatches)
        break;
    }

    size_t nintersect = nHashes - (mismatches / 2);
    int nunion = 2 * nHashes - nintersect;
    return 1.f - (nintersect / (float)nunion);
  }
};

template <class T> class Fingerprint {
public:
  uint64_t magnitude{0};
  static const size_t MaxOpcode = 68;
  std::array<uint32_t, MaxOpcode> OpcodeFreq;

  Fingerprint() = default;

  Fingerprint(T owner) {
    assert(owner != nullptr);
    for (size_t i = 0; i < MaxOpcode; i++)
      OpcodeFreq[i] = 0;
    for (Instruction &I : getInstructions(owner)) {
      if (I.getOpcode() > MaxOpcode || I.getOpcode() < 0) {
        errs() << "Opcode is " << I.getOpcode() << "\n";
        exit(123);
      }
      OpcodeFreq[I.getOpcode()]++;
      if (I.isTerminator())
        OpcodeFreq[0] += I.getNumSuccessors();
    }
    for (size_t i = 0; i < MaxOpcode; i++) {
      uint64_t val = OpcodeFreq[i];
      magnitude += val * val;
    }
  }

  uint32_t footprint() const { return sizeof(int) * MaxOpcode; }

  float distance(const Fingerprint &FP2) const {
    int Distance = 0;
    for (size_t i = 0; i < MaxOpcode; i++) {
      int Freq1 = OpcodeFreq[i];
      int Freq2 = FP2.OpcodeFreq[i];
      Distance += std::abs(Freq1 - Freq2);
    }
    return static_cast<float>(Distance);
  }
};

class BlockFingerprint : public Fingerprint<BasicBlock *> {
public:
  BasicBlock *BB{nullptr};
  size_t Size{0};

  BlockFingerprint(BasicBlock *BB) : Fingerprint(BB), BB(BB) {
    for (Instruction &I : *BB) {
      if (!isa<LandingPadInst>(&I) && !isa<PHINode>(&I)) {
        Size++;
      }
    }
  }
};

template <class T> class MatchInfo {
public:
  T candidate{nullptr};
  size_t Size{0};
  size_t OtherSize{0};
  size_t MergedSize{0};
  size_t Magnitude{0};
  size_t OtherMagnitude{0};
  float Distance{0};
  bool Valid{false};
  bool Profitable{false};

  MatchInfo() = default;
  MatchInfo(T candidate) : candidate(candidate){};
  MatchInfo(T candidate, size_t Size) : candidate(candidate), Size(Size){};
};

template <class T> class Matcher {
public:
  Matcher() = default;
  virtual ~Matcher() = default;

  virtual void add_candidate(T candidate, size_t size) = 0;
  virtual void remove_candidate(T candidate) = 0;
  virtual T next_candidate() = 0;
  virtual std::vector<MatchInfo<T>> &get_matches(T candidate) = 0;
  virtual size_t size() = 0;
  virtual void print_stats() = 0;
};

template <class T, template <typename> class FPTy = Fingerprint>
class MatcherManual : public Matcher<T> {
private:
  struct MatcherEntry {
    T candidate;
    size_t size;
    FPTy<T> FP;
    MatcherEntry() : MatcherEntry(nullptr, 0){};

    template <typename T1 = FPTy<T>, typename T2 = Fingerprint<T>>
    MatcherEntry(
        T candidate, size_t size,
        typename std::enable_if_t<std::is_same<T1, T2>::value, int> * = nullptr)
        : candidate(candidate), size(size), FP(candidate) {}

    template <typename T1 = FPTy<T>, typename T2 = FingerprintMH<T>>
    MatcherEntry(
        T candidate, size_t size, SearchStrategy &strategy,
        typename std::enable_if_t<std::is_same<T1, T2>::value, int> * = nullptr)
        : candidate(candidate), size(size), FP(candidate, strategy) {}
  };
  using MatcherIt = typename std::list<MatcherEntry>::iterator;

  bool initialized{false};
  FunctionMerger &FM;
  FunctionMergingOptions &Options;
  std::list<MatcherEntry> candidates;
  std::unordered_map<T, MatcherIt> cache;
  std::vector<MatchInfo<T>> matches;
  std::unordered_map<std::string, std::string> matchNames;

public:
  MatcherManual() = default;
  MatcherManual(FunctionMerger &FM, FunctionMergingOptions &Options,
                std::string Filename)
      : FM(FM), Options(Options) {
    std::ifstream File{Filename};
    std::string FuncName1, FuncName2;
    while (File >> FuncName1 >> FuncName2) {
      matchNames[FuncName1] = FuncName2;
      matchNames[FuncName2] = FuncName1;
    }
  }

  virtual ~MatcherManual() = default;

  void add_candidate(T candidate, size_t size) override {
    if (matchNames.count(GetValueName(candidate)) == 0)
      return;
    add_candidate_helper(candidate, size);
    cache[candidate] = candidates.begin();
  }

  template <typename T1 = FPTy<T>, typename T2 = Fingerprint<T>>
  void add_candidate_helper(
      T candidate, size_t size,
      typename std::enable_if_t<std::is_same<T1, T2>::value, int> * = nullptr) {
    candidates.emplace_front(candidate, size);
  }

  void remove_candidate(T candidate) override {
    auto cache_it = cache.find(candidate);
    assert(cache_it != cache.end());
    candidates.erase(cache_it->second);
  }

  T next_candidate() override {
    if (!initialized) {
      candidates.sort([&](auto &item1, auto &item2) -> bool {
        return item1.FP.magnitude > item2.FP.magnitude;
      });
      initialized = true;
    }
    update_matches(candidates.begin());
    return candidates.front().candidate;
  }

  std::vector<MatchInfo<T>> &get_matches(T candidate) override {
    return matches;
  }

  size_t size() override { return candidates.size(); }

  void print_stats() override {

    float MinDistance = std::numeric_limits<float>::max();
    float MaxDistance = 0;

    int Index1 = 0;
    for (auto It1 = candidates.begin(), E1 = candidates.end(); It1 != E1;
         It1++) {

      int BestIndex = 0;
      bool FoundCandidate = false;
      float BestDist = std::numeric_limits<float>::max();

      unsigned CountCandidates = 0;
      int Index2 = Index1;
      for (auto It2 = It1, E2 = candidates.end(); It2 != E2; It2++) {

        if (It1->candidate == It2->candidate || Index1 == Index2) {
          Index2++;
          continue;
        }

        if ((!FM.validMergeTypes(It1->candidate, It2->candidate, Options) &&
             !Options.EnableUnifiedReturnType) ||
            !validMergePair(It1->candidate, It2->candidate))
          continue;

        auto Dist = It1->FP.distance(It2->FP);
        if (Dist < BestDist) {
          BestDist = Dist;
          FoundCandidate = true;
          BestIndex = Index2;
        }
        if (RankingThreshold && CountCandidates > RankingThreshold) {
          break;
        }
        CountCandidates++;
        Index2++;
      }
      if (FoundCandidate) {
        int Distance = std::abs(Index1 - BestIndex);

        if (Distance > MaxDistance)
          MaxDistance = Distance;
        if (Distance < MinDistance)
          MinDistance = Distance;
      }
      Index1++;
    }
  }

private:
  void update_matches(MatcherIt it) {
    matches.clear();

    MatchInfo<T> best_match;
    best_match.OtherSize = it->size;
    best_match.OtherMagnitude = it->FP.magnitude;
    best_match.Distance = std::numeric_limits<float>::max();

    for (auto entry = std::next(candidates.cbegin());
         entry != candidates.cend(); ++entry) {
      if ((!FM.validMergeTypes(it->candidate, entry->candidate, Options) &&
           !Options.EnableUnifiedReturnType) ||
          !validMergePair(it->candidate, entry->candidate))
        continue;
      if (matchNames[GetValueName(it->candidate)] ==
          GetValueName(entry->candidate)) {
        best_match.candidate = entry->candidate;
        best_match.Size = entry->size;
        best_match.Magnitude = entry->FP.magnitude;
        best_match.Distance = 0;
        break;
      }
    }

    if (best_match.candidate != nullptr)
      matches.push_back(std::move(best_match));
    return;
  }
};

template <class T, template <typename> class FPTy = Fingerprint>
class MatcherFQ : public Matcher<T> {
private:
  struct MatcherEntry {
    T candidate;
    size_t size;
    FPTy<T> FP;
    MatcherEntry() : MatcherEntry(nullptr, 0){};

    template <typename T1 = FPTy<T>, typename T2 = Fingerprint<T>>
    MatcherEntry(
        T candidate, size_t size,
        typename std::enable_if_t<std::is_same<T1, T2>::value, int> * = nullptr)
        : candidate(candidate), size(size), FP(candidate) {}

    template <typename T1 = FPTy<T>, typename T2 = FingerprintMH<T>>
    MatcherEntry(
        T candidate, size_t size, SearchStrategy &strategy,
        typename std::enable_if_t<std::is_same<T1, T2>::value, int> * = nullptr)
        : candidate(candidate), size(size), FP(candidate, strategy) {}
  };
  using MatcherIt = typename std::list<MatcherEntry>::iterator;

  bool initialized{false};
  FunctionMerger &FM;
  FunctionMergingOptions &Options;
  std::list<MatcherEntry> candidates;
  std::unordered_map<T, MatcherIt> cache;
  std::vector<MatchInfo<T>> matches;
  SearchStrategy strategy;

public:
  MatcherFQ() = default;
  MatcherFQ(FunctionMerger &FM, FunctionMergingOptions &Options,
            size_t rows = 2, size_t bands = 100)
      : FM(FM), Options(Options), strategy(rows, bands){};

  virtual ~MatcherFQ() = default;

  void add_candidate(T candidate, size_t size) override {
    add_candidate_helper(candidate, size);
    cache[candidate] = candidates.begin();
  }

  template <typename T1 = FPTy<T>, typename T2 = Fingerprint<T>>
  void add_candidate_helper(
      T candidate, size_t size,
      typename std::enable_if_t<std::is_same<T1, T2>::value, int> * = nullptr) {
    candidates.emplace_front(candidate, size);
  }

  template <typename T1 = FPTy<T>, typename T2 = Fingerprint<T>>
  void add_candidate_helper(
      T candidate, size_t size,
      typename std::enable_if_t<!std::is_same<T1, T2>::value, int> * =
          nullptr) {
    candidates.emplace_front(candidate, size, strategy);
  }

  void remove_candidate(T candidate) override {
    auto cache_it = cache.find(candidate);
    assert(cache_it != cache.end());
    candidates.erase(cache_it->second);
  }

  T next_candidate() override {
    if (!initialized) {
      candidates.sort([&](auto &item1, auto &item2) -> bool {
        return item1.FP.magnitude > item2.FP.magnitude;
      });
      initialized = true;
    }
    update_matches(candidates.begin());
    return candidates.front().candidate;
  }

  std::vector<MatchInfo<T>> &get_matches(T candidate) override {
    return matches;
  }

  size_t size() override { return candidates.size(); }

  void print_stats() override {

    float MinDistance = std::numeric_limits<float>::max();
    float MaxDistance = 0;

    int Index1 = 0;
    for (auto It1 = candidates.begin(), E1 = candidates.end(); It1 != E1;
         It1++) {

      int BestIndex = 0;
      bool FoundCandidate = false;
      float BestDist = std::numeric_limits<float>::max();

      unsigned CountCandidates = 0;
      int Index2 = Index1;
      for (auto It2 = It1, E2 = candidates.end(); It2 != E2; It2++) {

        if (It1->candidate == It2->candidate || Index1 == Index2) {
          Index2++;
          continue;
        }

        if ((!FM.validMergeTypes(It1->candidate, It2->candidate, Options) &&
             !Options.EnableUnifiedReturnType) ||
            !validMergePair(It1->candidate, It2->candidate))
          continue;

        auto Dist = It1->FP.distance(It2->FP);
        if (Dist < BestDist) {
          BestDist = Dist;
          FoundCandidate = true;
          BestIndex = Index2;
        }
        if (RankingThreshold && CountCandidates > RankingThreshold) {
          break;
        }
        CountCandidates++;
        Index2++;
      }
      if (FoundCandidate) {
        int Distance = std::abs(Index1 - BestIndex);

        if (Distance > MaxDistance)
          MaxDistance = Distance;
        if (Distance < MinDistance)
          MinDistance = Distance;
      }
      Index1++;
    }
  }

private:
  void update_matches(MatcherIt it) {
    size_t CountCandidates = 0;
    matches.clear();

    MatchInfo<T> best_match;
    best_match.OtherSize = it->size;
    best_match.OtherMagnitude = it->FP.magnitude;
    best_match.Distance = std::numeric_limits<float>::max();

    if (ExplorationThreshold == 1) {
      for (auto entry = std::next(candidates.cbegin());
           entry != candidates.cend(); ++entry) {
        if ((!FM.validMergeTypes(it->candidate, entry->candidate, Options) &&
             !Options.EnableUnifiedReturnType) ||
            !validMergePair(it->candidate, entry->candidate))
          continue;
        auto new_distance = it->FP.distance(entry->FP);
        if (new_distance < best_match.Distance) {
          best_match.candidate = entry->candidate;
          best_match.Size = entry->size;
          best_match.Magnitude = entry->FP.magnitude;
          best_match.Distance = new_distance;
        }
        if (RankingThreshold && (CountCandidates > RankingThreshold))
          break;
        CountCandidates++;
      }
      if (best_match.candidate != nullptr)
        if (!EnableF3M || best_match.Distance < RankingDistance)
          matches.push_back(std::move(best_match));
      return;
    }

    for (auto &entry : candidates) {
      if (entry.candidate == it->candidate)
        continue;
      if ((!FM.validMergeTypes(it->candidate, entry.candidate, Options) &&
           !Options.EnableUnifiedReturnType) ||
          !validMergePair(it->candidate, entry.candidate))
        continue;
      MatchInfo<T> new_match(entry.candidate, entry.size);
      new_match.Distance = it->FP.distance(entry.FP);
      new_match.OtherSize = it->size;
      new_match.OtherMagnitude = it->FP.magnitude;
      new_match.Magnitude = entry.FP.magnitude;
      if (!EnableF3M || new_match.Distance < RankingDistance)
        matches.push_back(std::move(new_match));
      if (RankingThreshold && (CountCandidates > RankingThreshold))
        break;
      CountCandidates++;
    }

    if (ExplorationThreshold < matches.size()) {
      std::partial_sort(matches.begin(), matches.begin() + ExplorationThreshold,
                        matches.end(), [&](auto &match1, auto &match2) -> bool {
                          return match1.Distance < match2.Distance;
                        });
      matches.resize(ExplorationThreshold);
      std::reverse(matches.begin(), matches.end());
    } else {
      std::sort(matches.begin(), matches.end(),
                [&](auto &match1, auto &match2) -> bool {
                  return match1.Distance > match2.Distance;
                });
    }
  }
};

template <class T> class MatcherLSH : public Matcher<T> {
private:
  struct MatcherEntry {
    T candidate;
    size_t size;
    FingerprintMH<T> FP;
    MatcherEntry() : MatcherEntry(nullptr, 0){};
    MatcherEntry(T candidate, size_t size, SearchStrategy &strategy)
        : candidate(candidate), size(size), FP(candidate, strategy){};
  };
  using MatcherIt = typename std::list<MatcherEntry>::iterator;

  bool initialized{false};
  const size_t rows{2};
  const size_t bands{100};
  FunctionMerger &FM;
  FunctionMergingOptions &Options;
  SearchStrategy strategy;

  std::list<MatcherEntry> candidates;
  std::unordered_map<uint32_t, std::vector<MatcherIt>> lsh;
  std::vector<std::pair<T, MatcherIt>> cache;
  std::vector<MatchInfo<T>> matches;

public:
  MatcherLSH() = default;
  MatcherLSH(FunctionMerger &FM, FunctionMergingOptions &Options, size_t rows,
             size_t bands)
      : rows(rows), bands(bands), FM(FM), Options(Options),
        strategy(rows, bands){};

  virtual ~MatcherLSH() = default;

  void add_candidate(T candidate, size_t size) override {
    candidates.emplace_front(candidate, size, strategy);

    auto it = candidates.begin();
    auto &bandHash = it->FP.bandHash;
    for (size_t i = 0; i < bands; ++i) {
      if (lsh.count(bandHash[i]) > 0)
        lsh.at(bandHash[i]).push_back(it);
      else
        lsh.insert(std::make_pair(bandHash[i], std::vector<MatcherIt>(1, it)));
    }
  }

  void remove_candidate(T candidate) override {
    auto cache_it = candidates.end();
    for (auto &cache_item : cache) {
      if (cache_item.first == candidate) {
        cache_it = cache_item.second;
        break;
      }
    }
    assert(cache_it != candidates.end());

    auto &FP = cache_it->FP;
    for (size_t i = 0; i < bands; ++i) {
      if (lsh.count(FP.bandHash[i]) == 0)
        continue;

      auto &foundFs = lsh.at(FP.bandHash[i]);
      for (size_t j = 0; j < foundFs.size(); ++j)
        if (foundFs[j]->candidate == candidate)
          lsh.at(FP.bandHash[i]).erase(lsh.at(FP.bandHash[i]).begin() + j);
    }
    candidates.erase(cache_it);
  }

  T next_candidate() override {
    if (!initialized) {
      candidates.sort([&](auto &item1, auto &item2) -> bool {
        return item1.FP.magnitude > item2.FP.magnitude;
      });
      initialized = true;
    }
    update_matches(candidates.begin());
    return candidates.front().candidate;
  }

  std::vector<MatchInfo<T>> &get_matches(T candidate) override {
    return matches;
  }

  size_t size() override { return candidates.size(); }

  void print_stats() override {
    std::unordered_set<T> seen;
    std::vector<uint32_t> hist_bucket_size(20);
    std::vector<uint32_t> hist_distances(21);
    std::vector<uint32_t> hist_distances_diff(21);
    uint32_t duplicate_hashes = 0;

    for (auto it = lsh.cbegin(); it != lsh.cend(); ++it) {
      size_t idx = 31 - __builtin_clz(it->second.size());
      idx = idx < 20 ? idx : 19;
      hist_bucket_size[idx]++;
    }
    for (size_t i = 0; i < 20; i++)
      errs() << "STATS: Histogram Bucket Size " << (1 << i) << " : "
             << hist_bucket_size[i] << "\n";
    return;

    for (auto it = candidates.begin(); it != candidates.end(); ++it) {
      seen.clear();
      seen.reserve(candidates.size() / 10);

      float best_distance = std::numeric_limits<float>::max();
      std::unordered_set<uint32_t> temp(it->FP.hash.begin(), it->FP.hash.end());
      duplicate_hashes += it->FP.hash.size() - temp.size();

      for (size_t i = 0; i < bands; ++i) {
        auto &foundFs = lsh.at(it->FP.bandHash[i]);
        size_t idx = 31 - __builtin_clz(foundFs.size());
        idx = idx < 20 ? idx : 19;
        hist_bucket_size[idx]++;
        for (size_t j = 0; j < foundFs.size(); ++j) {
          auto match_it = foundFs[j];
          if ((match_it->candidate == NULL) ||
              (match_it->candidate == it->candidate))
            continue;
          if ((!FM.validMergeTypes(it->candidate, match_it->candidate,
                                   Options) &&
               !Options.EnableUnifiedReturnType) ||
              !validMergePair(it->candidate, match_it->candidate))
            continue;

          if (seen.count(match_it->candidate) == 1)
            continue;
          seen.insert(match_it->candidate);

          auto distance = it->FP.distance(match_it->FP);
          best_distance = distance < best_distance ? distance : best_distance;
          auto idx2 = static_cast<size_t>(distance * 20);
          idx2 = idx2 < 21 ? idx2 : 20;
          hist_distances[idx2]++;
          auto idx3 = static_cast<size_t>((distance - best_distance) * 20);
          idx3 = idx3 < 21 ? idx3 : 20;
          hist_distances_diff[idx3]++;
        }
      }
    }
  }

private:
  void update_matches(MatcherIt it) {
    size_t CountCandidates = 0;
    std::unordered_set<T> seen;
    seen.reserve(candidates.size() / 10);
    matches.clear();
    cache.clear();
    cache.emplace_back(it->candidate, it);

    auto &FP = it->FP;
    MatchInfo<T> best_match;
    best_match.Distance = std::numeric_limits<float>::max();
    for (size_t i = 0; i < bands; ++i) {
      assert(lsh.count(FP.bandHash[i]) > 0);

      auto &foundFs = lsh.at(FP.bandHash[i]);
      for (size_t j = 0; j < foundFs.size() && j < BucketSizeCap; ++j) {
        auto match_it = foundFs[j];
        if ((match_it->candidate == NULL) ||
            (match_it->candidate == it->candidate))
          continue;
        if ((!FM.validMergeTypes(it->candidate, match_it->candidate, Options) &&
             !Options.EnableUnifiedReturnType) ||
            !validMergePair(it->candidate, match_it->candidate))
          continue;

        if (seen.count(match_it->candidate) == 1)
          continue;
        seen.insert(match_it->candidate);

        MatchInfo<T> new_match(match_it->candidate, match_it->size);
        if (best_match.Distance < 0.1)
          new_match.Distance =
              FP.distance_under(match_it->FP, best_match.Distance);
        else
          new_match.Distance = FP.distance(match_it->FP);
        new_match.OtherSize = it->size;
        new_match.OtherMagnitude = FP.magnitude;
        new_match.Magnitude = match_it->FP.magnitude;
        if (new_match.Distance < best_match.Distance &&
            new_match.Distance < RankingDistance)
          best_match = new_match;
        if (ExplorationThreshold > 1)
          if (new_match.Distance < RankingDistance)
            matches.push_back(new_match);
        cache.emplace_back(match_it->candidate, match_it);
        if (RankingThreshold && (CountCandidates > RankingThreshold))
          break;
        CountCandidates++;
      }
      // If we've gone through i = 0 without finding a distance of 0.0
      // the minimum distance we might ever find is 2.0 / (nHashes + 1)
      if ((ExplorationThreshold == 1) &&
          (best_match.Distance < (2.0 / (rows * bands))))
        break;
      if (RankingThreshold && (CountCandidates > RankingThreshold))
        break;
    }

    if (ExplorationThreshold == 1)
      if (best_match.candidate != nullptr)
        matches.push_back(std::move(best_match));

    if (matches.size() <= 1)
      return;

    size_t toRank = std::min((size_t)ExplorationThreshold, matches.size());

    std::partial_sort(matches.begin(), matches.begin() + toRank, matches.end(),
                      [&](auto &match1, auto &match2) -> bool {
                        return match1.Distance < match2.Distance;
                      });
    matches.resize(toRank);
    std::reverse(matches.begin(), matches.end());
  }
};

template <class T> class MatcherReport {
private:
  struct MatcherEntry {
    T candidate;
    Fingerprint<T> FPF;
    FingerprintMH<T> FPMH;
    MatcherEntry(T candidate, SearchStrategy &strategy)
        : candidate(candidate), FPF(candidate), FPMH(candidate, strategy){};
  };
  using MatcherIt = typename std::list<MatcherEntry>::iterator;

  FunctionMerger &FM;
  FunctionMergingOptions &Options;
  SearchStrategy strategy;
  std::vector<MatcherEntry> candidates;

public:
  MatcherReport() = default;
  MatcherReport(size_t rows, size_t bands, FunctionMerger &FM,
                FunctionMergingOptions &Options)
      : FM(FM), Options(Options), strategy(rows, bands){};

  ~MatcherReport() = default;

  void add_candidate(T candidate) {
    candidates.emplace_back(candidate, strategy);
  }

  void report() const {
    char distance_mh_str[20];

    for (auto &entry : candidates) {
      uint64_t val = 0;
      for (auto &num : entry.FPF.OpcodeFreq)
        val += num;
    }

    std::string Name("_m_f_");
    for (auto it1 = candidates.cbegin(); it1 != candidates.cend(); ++it1) {
      for (auto it2 = std::next(it1); it2 != candidates.cend(); ++it2) {
        if ((!FM.validMergeTypes(it1->candidate, it2->candidate, Options) &&
             !Options.EnableUnifiedReturnType) ||
            !validMergePair(it1->candidate, it2->candidate))
          continue;

        auto distance_fq = it1->FPF.distance(it2->FPF);
        auto distance_mh = it1->FPMH.distance(it2->FPMH);
        std::snprintf(distance_mh_str, 20, "%.5f", distance_mh);

        FunctionMergeResult Result =
            FM.merge(it1->candidate, it2->candidate, Name, Options);
      }
    }
  }
};

AlignedCode::AlignedCode(BasicBlock *BB1, BasicBlock *BB2) {
  // this should never happen
  assert(BB1 != nullptr || BB2 != nullptr);

  // Add only BB1, skipping Phi nodes and Landing Pads
  if (BB1 != nullptr && BB2 == nullptr) {
    Data.emplace_back(BB1, nullptr, false);
    for (Instruction &I : *BB1) {
      if (isa<PHINode>(&I) || isa<LandingPadInst>(&I))
        continue;
      Data.emplace_back(&I, nullptr, false);
    }
    return;
  }

  // Add only BB2, skipping Phi nodes and Landing Pads
  if (BB1 == nullptr && BB2 != nullptr) {
    Data.emplace_back(nullptr, BB2, false);
    for (Instruction &I : *BB2) {
      if (isa<PHINode>(&I) || isa<LandingPadInst>(&I))
        continue;
      Data.emplace_back(nullptr, &I, false);
    }
    return;
  }

  // Add both, skipping Phi nodes and Landing Pads
  Data.emplace_back(BB1, BB2, FunctionMerger::matchBlocks(BB1, BB2));

  auto It1 = BB1->begin();
  while (isa<PHINode>(*It1) || isa<LandingPadInst>(*It1))
    It1++;

  auto It2 = BB2->begin();
  while (isa<PHINode>(*It2) || isa<LandingPadInst>(*It2))
    It2++;

  while (It1 != BB1->end() && It2 != BB2->end()) {
    Instruction *I1 = &*It1;
    Instruction *I2 = &*It2;

    if (FunctionMerger::matchInstructions(I1, I2)) {
      Data.emplace_back(I1, I2, true);
    } else {
      Data.emplace_back(I1, nullptr, false);
      Data.emplace_back(nullptr, I2, false);
    }

    It1++;
    It2++;
  }
  assert((It1 == BB1->end()) && (It2 == BB2->end()));
}

bool AlignedCode::isProfitable() const {
  int OriginalCost = 0;
  int MergedCost = 0;

  bool InsideSplit = false;

  for (auto &Entry : Data) {
    Instruction *I1 = nullptr;
    if (Entry.get(0))
      I1 = dyn_cast<Instruction>(Entry.get(0));

    Instruction *I2 = nullptr;
    if (Entry.get(1))
      I2 = dyn_cast<Instruction>(Entry.get(1));

    bool IsInstruction = I1 != nullptr || I2 != nullptr;
    if (Entry.match()) {
      if (IsInstruction) {
        OriginalCost += 2;
        MergedCost += 1;
      }
      if (InsideSplit) {
        InsideSplit = false;
        MergedCost += 2;
      }
    } else {
      if (IsInstruction) {
        OriginalCost += 1;
        MergedCost += 1;
      }
      if (!InsideSplit) {
        InsideSplit = true;
        MergedCost += 1;
      }
    }
  }

  bool Profitable = (MergedCost <= OriginalCost);
  return Profitable;
}

void AlignedCode::extend(const AlignedCode &Other) {
  for (auto &Entry : Other) {
    Instruction *I1 = nullptr;
    if (Entry.get(0))
      I1 = dyn_cast<Instruction>(Entry.get(0));

    Instruction *I2 = nullptr;
    if (Entry.get(1))
      I2 = dyn_cast<Instruction>(Entry.get(1));

    bool IsInstruction = I1 != nullptr || I2 != nullptr;

    Data.emplace_back(Entry.get(0), Entry.get(1), Entry.match());

    if (IsInstruction) {
      Insts++;
      if (Entry.match()) {
        Matches++;
        Instruction *I = I1 ? I1 : I2;
        if (!I->isTerminator())
          CoreMatches++;
      }
    }
  }
}

bool AcrossBlocks;

FunctionMergeResult
FunctionMerger::merge(Function *F1, Function *F2, std::string Name,
                      const FunctionMergingOptions &Options) {
  bool ProfitableFn = true;
  LLVMContext &Context = *ContextPtr;
  FunctionMergeResult ErrorResponse(F1, F2, nullptr);

  if (!validMergePair(F1, F2))
    return ErrorResponse;

  AlignedCode AlignedSeq;
  NeedlemanWunschSA<SmallVectorImpl<Value *>> SA(ScoringSystem(-1, 2),
                                                 FunctionMerger::match);

  if (EnableHyFMNW || EnableHyFMPA) { // Processing individual pairs of blocks

    // Fingerprints for all Blocks in F1 organized by size
    std::map<size_t, std::vector<BlockFingerprint>> Blocks;
    for (BasicBlock &BB1 : *F1) {
      BlockFingerprint BD1(&BB1);

      Blocks[BD1.Size].push_back(std::move(BD1));
    }

    for (BasicBlock &BIt : *F2) {
      BasicBlock *BB2 = &BIt;
      BlockFingerprint BD2(BB2);

      // list all the map entries in Blocks in order of distance from BD2.Size
      auto ItSetIncr = Blocks.lower_bound(BD2.Size);

      auto ItSetDecr = std::reverse_iterator<
          std::map<size_t, std::vector<BlockFingerprint>>::iterator>(ItSetIncr);

      std::vector<decltype(ItSetIncr)> ItSets;

      if (EnableHyFMNW) {
        while (ItSetDecr != Blocks.rend() && ItSetIncr != Blocks.end()) {
          if (BD2.Size - ItSetDecr->first < ItSetIncr->first - BD2.Size) {
            ItSets.push_back(std::prev(ItSetDecr.base()));
            ItSetDecr++;
          } else {
            ItSets.push_back(ItSetIncr);
            ItSetIncr++;
          }
        }

        while (ItSetDecr != Blocks.rend()) {
          ItSets.push_back(std::prev(ItSetDecr.base()));
          ItSetDecr++;
        }

        while (ItSetIncr != Blocks.end()) {
          ItSets.push_back(ItSetIncr);
          ItSetIncr++;
        }
      } else {
        ItSetIncr = Blocks.find(BD2.Size);
        if (ItSetIncr != Blocks.end())
          ItSets.push_back(ItSetIncr);
      }

      // Find the closest block starting from blocks with similar size
      std::vector<BlockFingerprint>::iterator BestIt;
      std::map<size_t, std::vector<BlockFingerprint>>::iterator BestSet;
      float BestDist = std::numeric_limits<float>::max();

      for (auto ItSet : ItSets) {
        for (auto BDIt = ItSet->second.begin(), E = ItSet->second.end();
             BDIt != E; BDIt++) {
          auto D = BD2.distance(*BDIt);
          if (D < BestDist) {
            BestDist = D;
            BestIt = BDIt;
            BestSet = ItSet;
            if (BestDist < std::numeric_limits<float>::epsilon())
              break;
          }
        }
        if (BestDist < std::numeric_limits<float>::epsilon())
          break;
      }

      bool MergedBlock = false;
      if (BestDist < std::numeric_limits<float>::max()) {
        BasicBlock *BB1 = BestIt->BB;
        AlignedCode AlignedBlocks;

        if (EnableHyFMNW) {
          SmallVector<Value *, 8> BB1Vec;
          vectorizeBB(BB1Vec, BB1);

          SmallVector<Value *, 8> BB2Vec;
          vectorizeBB(BB2Vec, BB2);

          AlignedBlocks = SA.getAlignment(BB1Vec, BB2Vec);

        } else if (EnableHyFMPA) {
          AlignedBlocks = AlignedCode(BB1, BB2);
        }

        if (!HyFMProfitability || AlignedBlocks.isProfitable()) {
          AlignedSeq.extend(AlignedBlocks);
          BestSet->second.erase(BestIt);
          MergedBlock = true;
        }
      }

      if (!MergedBlock)
        AlignedSeq.extend(AlignedCode(nullptr, BB2));
    }

    for (auto &Pair : Blocks)
      for (auto &BD1 : Pair.second)
        AlignedSeq.extend(AlignedCode(BD1.BB, nullptr));

    ProfitableFn = AlignedSeq.hasMatches();
  }

  if (!ProfitableFn && !ReportStats) {

    return ErrorResponse;
  }

  AcrossBlocks = false;
  BasicBlock *CurrBB0 = nullptr;
  BasicBlock *CurrBB1 = nullptr;
  for (auto &Entry : AlignedSeq) {
    if (Entry.match()) {
      if (isa<BasicBlock>(Entry.get(1))) {
        CurrBB1 = cast<BasicBlock>(Entry.get(1));
      } else if (auto *I = dyn_cast<Instruction>(Entry.get(1))) {
        if (CurrBB1 == nullptr)
          CurrBB1 = I->getParent();
        else if (CurrBB1 != I->getParent()) {
          AcrossBlocks = true;
        }
      }
      if (isa<BasicBlock>(Entry.get(0))) {
        CurrBB0 = cast<BasicBlock>(Entry.get(0));
      } else if (auto *I = dyn_cast<Instruction>(Entry.get(0))) {
        if (CurrBB0 == nullptr)
          CurrBB0 = I->getParent();
        else if (CurrBB0 != I->getParent()) {
          AcrossBlocks = true;
        }
      }
    } else {
      if (isa_and_nonnull<BasicBlock>(Entry.get(0)))
        CurrBB1 = nullptr;
      if (isa_and_nonnull<BasicBlock>(Entry.get(1)))
        CurrBB0 = nullptr;
    }
  }
  if (AcrossBlocks) {
  }

  // errs() << "Code Gen\n";

  // errs() << "Creating function type\n";

  // Merging parameters
  std::map<unsigned, unsigned> ParamMap1;
  std::map<unsigned, unsigned> ParamMap2;
  std::vector<Type *> Args;

  // errs() << "Merging arguments\n";
  MergeArguments(Context, F1, F2, AlignedSeq, ParamMap1, ParamMap2, Args,
                 Options);

  Type *RetType1 = F1->getReturnType();
  Type *RetType2 = F2->getReturnType();
  Type *ReturnType = nullptr;

  bool RequiresUnifiedReturn = false;

  if (validMergeTypes(F1, F2, Options)) {
    ReturnType = RetType1;
    if (ReturnType->isVoidTy()) {
      ReturnType = RetType2;
    }
  } else if (Options.EnableUnifiedReturnType) {
    RequiresUnifiedReturn = true;

    auto SizeOfTy1 = DL->getTypeStoreSize(RetType1);
    auto SizeOfTy2 = DL->getTypeStoreSize(RetType2);
    if (SizeOfTy1 >= SizeOfTy2) {
      ReturnType = RetType1;
    } else {
      ReturnType = RetType2;
    }
  } else {
    return ErrorResponse;
  }
  FunctionType *FTy =
      FunctionType::get(ReturnType, ArrayRef<Type *>(Args), false);

  if (Name.empty()) {
    Name = "_m_f";
  }
  Function *MergedFunc =
      Function::Create(FTy, // GlobalValue::LinkageTypes::InternalLinkage,
                       GlobalValue::LinkageTypes::PrivateLinkage, Twine(Name),
                       M); // merged.function

  ValueToValueMapTy VMap;

  std::vector<Argument *> ArgsList;
  for (Argument &arg : MergedFunc->args()) {
    ArgsList.push_back(&arg);
  }
  Value *FuncId = ArgsList[0];

  int ArgId = 0;
  for (auto I = F1->arg_begin(), E = F1->arg_end(); I != E; I++) {
    VMap[&(*I)] = ArgsList[ParamMap1[ArgId]];

    ArgId++;
  }

  ArgId = 0;
  for (auto I = F2->arg_begin(), E = F2->arg_end(); I != E; I++) {
    VMap[&(*I)] = ArgsList[ParamMap2[ArgId]];

    ArgId++;
  }

  SetFunctionAttributes(F1, F2, MergedFunc);

  Value *IsFunc1 = FuncId;

  auto Gen = [&](auto &CG) {
    CG.setFunctionIdentifier(IsFunc1)
        .setEntryPoints(&F1->getEntryBlock(), &F2->getEntryBlock())
        .setReturnTypes(RetType1, RetType2)
        .setMergedFunction(MergedFunc)
        .setMergedEntryPoint(BasicBlock::Create(Context, "entry", MergedFunc))
        .setMergedReturnType(ReturnType, RequiresUnifiedReturn)
        .setContext(ContextPtr)
        .setIntPtrType(IntPtrTy);
    if (!CG.generate(AlignedSeq, VMap, Options)) {

      MergedFunc->eraseFromParent();
      MergedFunc = nullptr;
      if (Debug)
        errs() << "ERROR: Failed to generate the merged function!\n";
      return false;
    }
    return true;
  };

  SALSSACodeGen CG(F1, F2);
  if (!Gen(CG)) {
    if (Debug)
      errs() << "feisen\n";
    FunctionMergeResult Result(false);
    return Result;
  }

  FunctionMergeResult Result(F1, F2, MergedFunc, RequiresUnifiedReturn);
  Result.setArgumentMapping(F1, ParamMap1);
  Result.setArgumentMapping(F2, ParamMap2);
  Result.setFunctionIdArgument(FuncId != nullptr);
  return Result;
}

void FunctionMerger::replaceByCall(Function *F, FunctionMergeResult &MFR,
                                   const FunctionMergingOptions &Options) {
  LLVMContext &Context = M->getContext();

  Value *FuncId = MFR.getFunctionIdValue(F);
  Function *MergedF = MFR.getMergedFunction();

  // Make sure we preserve its linkage
  auto Linkage = F->getLinkage();

  F->deleteBody();
  BasicBlock *NewBB = BasicBlock::Create(Context, "", F);
  IRBuilder<> Builder(NewBB);

  std::vector<Value *> args;
  for (unsigned i = 0; i < MergedF->getFunctionType()->getNumParams(); i++) {
    args.push_back(nullptr);
  }

  if (MFR.hasFunctionIdArgument()) {
    args[0] = FuncId;
  }

  std::vector<Argument *> ArgsList;
  for (Argument &arg : F->args()) {
    ArgsList.push_back(&arg);
  }

  for (auto Pair : MFR.getArgumentMapping(F)) {
    args[Pair.second] = ArgsList[Pair.first];
  }

  for (unsigned i = 0; i < args.size(); i++) {
    if (args[i] == nullptr) {
      args[i] = UndefValue::get(MergedF->getFunctionType()->getParamType(i));
    }
  }

  F->setLinkage(Linkage);

  CallInst *CI =
      (CallInst *)Builder.CreateCall(MergedF, ArrayRef<Value *>(args));
  CI->setTailCall();
  CI->setCallingConv(MergedF->getCallingConv());
  CI->setAttributes(MergedF->getAttributes());
  CI->setIsNoInline();

  if (F->getReturnType()->isVoidTy()) {
    Builder.CreateRetVoid();
  } else {
    Value *CastedV;
    if (MFR.needUnifiedReturn()) {
      Value *AddrCI = Builder.CreateAlloca(CI->getType());
      Builder.CreateStore(CI, AddrCI);
      Value *CastedAddr = Builder.CreatePointerCast(
          AddrCI,
          PointerType::get(F->getReturnType(), DL->getAllocaAddrSpace()));
      CastedV = Builder.CreateLoad(F->getReturnType(), CastedAddr);
    } else {
      CastedV = createCastIfNeeded(CI, F->getReturnType(), Builder, IntPtrTy,
                                   Options);
    }
    Builder.CreateRet(CastedV);
  }
}

bool FunctionMerger::replaceCallsWith(Function *F, FunctionMergeResult &MFR,
                                      const FunctionMergingOptions &Options) {

  Value *FuncId = MFR.getFunctionIdValue(F);
  Function *MergedF = MFR.getMergedFunction();

  unsigned CountUsers = 0;
  std::vector<CallBase *> Calls;
  for (User *U : F->users()) {
    CountUsers++;
    if (auto *CI = dyn_cast<CallInst>(U)) {
      if (CI->getCalledFunction() == F) {
        Calls.push_back(CI);
      }
    } else if (auto *II = dyn_cast<InvokeInst>(U)) {
      if (II->getCalledFunction() == F) {
        Calls.push_back(II);
      }
    }
  }

  if (Calls.size() < CountUsers)
    return false;

  for (CallBase *CI : Calls) {
    IRBuilder<> Builder(CI);

    std::vector<Value *> args;
    for (unsigned i = 0; i < MergedF->getFunctionType()->getNumParams(); i++) {
      args.push_back(nullptr);
    }

    if (MFR.hasFunctionIdArgument()) {
      args[0] = FuncId;
    }

    for (auto Pair : MFR.getArgumentMapping(F)) {
      args[Pair.second] = CI->getArgOperand(Pair.first);
    }

    for (unsigned i = 0; i < args.size(); i++) {
      if (args[i] == nullptr) {
        args[i] = UndefValue::get(MergedF->getFunctionType()->getParamType(i));
      }
    }

    CallBase *NewCB = nullptr;
    if (CI->getOpcode() == Instruction::Call) {
      NewCB = (CallInst *)Builder.CreateCall(MergedF->getFunctionType(),
                                             MergedF, args);
    } else if (CI->getOpcode() == Instruction::Invoke) {
      auto *II = dyn_cast<InvokeInst>(CI);
      NewCB = (InvokeInst *)Builder.CreateInvoke(MergedF->getFunctionType(),
                                                 MergedF, II->getNormalDest(),
                                                 II->getUnwindDest(), args);
    }
    NewCB->setCallingConv(MergedF->getCallingConv());
    NewCB->setAttributes(MergedF->getAttributes());
    NewCB->setIsNoInline();
    Value *CastedV = NewCB;
    if (!F->getReturnType()->isVoidTy()) {
      if (MFR.needUnifiedReturn()) {
        Value *AddrCI = Builder.CreateAlloca(NewCB->getType());
        Builder.CreateStore(NewCB, AddrCI);
        Value *CastedAddr = Builder.CreatePointerCast(
            AddrCI,
            PointerType::get(F->getReturnType(), DL->getAllocaAddrSpace()));
        CastedV = Builder.CreateLoad(F->getReturnType(), CastedAddr);
      } else {
        CastedV = createCastIfNeeded(NewCB, F->getReturnType(), Builder,
                                     IntPtrTy, Options);
      }
    }

    if (CI->getNumUses() > 0) {
      CI->replaceAllUsesWith(CastedV);
    }
    CI->eraseFromParent();
  }

  return true;
}

static bool ShouldPreserveGV(const GlobalValue *GV) {
  // Function must be defined here
  if (GV->isDeclaration())
    return true;

  if (GV->hasDLLExportStorageClass())
    return true;

  if (GV->hasLocalLinkage())
    return false;

  return false;
}

static int RequiresOriginalInterface(Function *F, FunctionMergeResult &MFR,
                                     StringSet<> &AlwaysPreserved) {
  bool CanErase = !F->hasAddressTaken();
  CanErase =
      CanErase && (AlwaysPreserved.find(F->getName()) == AlwaysPreserved.end());
  if (!HasWholeProgram) {
    CanErase = CanErase && F->isDiscardableIfUnused();
  }
  return !CanErase;
}

static int RequiresOriginalInterfaces(FunctionMergeResult &MFR,
                                      StringSet<> &AlwaysPreserved) {
  auto FPair = MFR.getFunctions();
  Function *F1 = FPair.first;
  Function *F2 = FPair.second;
  return (RequiresOriginalInterface(F1, MFR, AlwaysPreserved) ? 1 : 0) +
         (RequiresOriginalInterface(F2, MFR, AlwaysPreserved) ? 1 : 0);
}

void FunctionMerger::updateCallGraph(Function *F, FunctionMergeResult &MFR,
                                     StringSet<> &AlwaysPreserved,
                                     const FunctionMergingOptions &Options) {
  replaceByCall(F, MFR, Options);
  if (!RequiresOriginalInterface(F, MFR, AlwaysPreserved)) {
    bool CanErase = replaceCallsWith(F, MFR, Options);
    CanErase = CanErase && F->use_empty();
    CanErase = CanErase &&
               (AlwaysPreserved.find(F->getName()) == AlwaysPreserved.end());
    if (!HasWholeProgram) {
      CanErase = CanErase && !ShouldPreserveGV(F);
      CanErase = CanErase && F->isDiscardableIfUnused();
    }
    if (CanErase)
      F->eraseFromParent();
  }
}

void FunctionMerger::updateCallGraph(FunctionMergeResult &MFR,
                                     StringSet<> &AlwaysPreserved,
                                     const FunctionMergingOptions &Options) {
  auto FPair = MFR.getFunctions();
  Function *F1 = FPair.first;
  Function *F2 = FPair.second;
  updateCallGraph(F1, MFR, AlwaysPreserved, Options);
  updateCallGraph(F2, MFR, AlwaysPreserved, Options);
}

static int EstimateThunkOverhead(FunctionMergeResult &MFR,
                                 StringSet<> &AlwaysPreserved) {
  return RequiresOriginalInterfaces(MFR, AlwaysPreserved) *
         (2 + MFR.getMergedFunction()->getFunctionType()->getNumParams());
}

static size_t EstimateFunctionSize(Function *F, TargetTransformInfo *TTI) {
  float size = 0;
  for (Instruction &I : instructions(F)) {
    switch (I.getOpcode()) {
    case Instruction::PHI:
      size += 0.2;
      break;
    default:
      auto cost = TTI->getInstructionCost(
          &I, TargetTransformInfo::TargetCostKind::TCK_CodeSize);
      size += cost.getValue().value();
    }
  }
  return size_t(std::ceil(size));
}

unsigned instToInt(Instruction *I) {
  uint32_t value = 0;
  static uint32_t pseudorand_value = 100;

  if (pseudorand_value > 10000)
    pseudorand_value = 100;

  value = I->getOpcode();

  uint32_t operands =
      I->getOpcode() == Instruction::Ret ? 1 : I->getNumOperands();
  value = value * (operands + 1);

  auto instTypeID = static_cast<uint32_t>(I->getType()->getTypeID());
  value = value * (instTypeID + 1);
  auto *ITypePtr = I->getType();
  if (ITypePtr) {
    value = value * (reinterpret_cast<std::uintptr_t>(ITypePtr) + 1);
  }

  for (size_t i = 0; i < I->getNumOperands(); i++) {
    auto operTypeID =
        static_cast<uint32_t>(I->getOperand(i)->getType()->getTypeID());
    value = value * (operTypeID + 1);

    auto *IOperTypePtr = I->getOperand(i)->getType();

    if (IOperTypePtr) {
      value =
          value *
          (reinterpret_cast<std::uintptr_t>(I->getOperand(i)->getType()) + 1);
    }

    value = value * (i + 1);
  }
  return value;

  // Now for the funky stuff -- this is gonna be a wild ride
  switch (I->getOpcode()) {

  case Instruction::Load: {

    const LoadInst *LI = dyn_cast<LoadInst>(I);
    uint32_t lValue = LI->isVolatile() ? 1 : 10;        // Volatility
    lValue += LI->getAlign().value();                   // Alignment
    lValue += static_cast<unsigned>(LI->getOrdering()); // Ordering

    value = value * lValue;

    break;
  }

  case Instruction::Store: {

    const StoreInst *SI = dyn_cast<StoreInst>(I);
    uint32_t sValue = SI->isVolatile() ? 2 : 20;        // Volatility
    sValue += SI->getAlign().value();                   // Alignment
    sValue += static_cast<unsigned>(SI->getOrdering()); // Ordering

    value = value * sValue;

    break;
  }

  case Instruction::Alloca: {
    const AllocaInst *AI = dyn_cast<AllocaInst>(I);
    uint32_t aValue = AI->getAlign().value(); // Alignment

    if (AI->getArraySize()) {
      aValue += reinterpret_cast<std::uintptr_t>(AI->getArraySize());
    }

    value = value * (aValue + 1);

    break;
  }

  case Instruction::GetElementPtr: // Important
  {

    auto *GEP = dyn_cast<GetElementPtrInst>(I);
    uint32_t gValue = 1;

    SmallVector<Value *, 8> Indices(GEP->idx_begin(), GEP->idx_end());
    gValue = Indices.size() + 1;

    gValue += GEP->isInBounds() ? 3 : 30;

    Type *AggTy = GEP->getSourceElementType();
    gValue += static_cast<unsigned>(AggTy->getTypeID());

    unsigned curIndex = 1;
    for (; curIndex != Indices.size(); ++curIndex) {

      if (!AggTy || AggTy->isPointerTy()) {
        if (Deterministic)
          value = pseudorand_value++;
        else
          value = std::rand() % 10000 + 100;
        break;
      }

      Value *Idx = Indices[curIndex];

      if (isa<StructType>(AggTy)) {
        if (!isa<ConstantInt>(Idx)) {
          if (Deterministic)
            value = pseudorand_value++;
          else
            value =
                std::rand() % 10000 + 100; // Use a random number as we don't
                                           // want this to match with anything
          break;
        }

        auto i = 0;
        if (Idx) {
          i = reinterpret_cast<std::uintptr_t>(Idx);
        }
        gValue += i;
      }
    }

    value = value * gValue;

    break;
  }

  case Instruction::Switch: {
    auto *SI = dyn_cast<SwitchInst>(I);
    uint32_t sValue = 1;
    sValue = SI->getNumCases();

    auto CaseIt = SI->case_begin(), CaseEnd = SI->case_end();

    while (CaseIt != CaseEnd) {
      auto *Case = &*CaseIt;
      if (Case) {
        sValue += reinterpret_cast<std::uintptr_t>(Case);
      }
      CaseIt++;
    }

    value = value * sValue;

    break;
  }

  case Instruction::Call: {
    auto *CI = dyn_cast<CallInst>(I);
    uint32_t cValue = 1;

    if (CI->isInlineAsm()) {
      if (Deterministic)
        value = pseudorand_value++;
      else
        value = std::rand() % 10000 + 100;
      break;
    }

    if (CI->getCalledFunction()) {
      cValue = reinterpret_cast<std::uintptr_t>(CI->getCalledFunction());
    }

    if (Function *F = CI->getCalledFunction()) {
      if (auto ID = (Intrinsic::ID)F->getIntrinsicID()) {
        cValue += static_cast<unsigned>(ID);
      }
    }

    cValue += static_cast<unsigned>(CI->getCallingConv());

    value = value * cValue;

    break;
  }

  case Instruction::Invoke: // Need to look at matching landing pads
  {
    auto *II = dyn_cast<InvokeInst>(I);
    uint32_t iValue = 1;

    iValue = static_cast<unsigned>(II->getCallingConv());

    if (II->getAttributes().getRawPointer()) {
      iValue +=
          reinterpret_cast<std::uintptr_t>(II->getAttributes().getRawPointer());
    }

    value = value * iValue;

    break;
  }

  case Instruction::InsertValue: {
    auto *IVI = dyn_cast<InsertValueInst>(I);

    uint32_t ivValue = 1;

    ivValue = IVI->getNumIndices();

    // check element wise equality
    auto Idx = IVI->getIndices();
    const auto *IdxIt = Idx.begin();
    const auto *IdxEnd = Idx.end();

    while (IdxIt != IdxEnd) {
      auto *val = &*IdxIt;
      if (val) {
        ivValue += reinterpret_cast<unsigned>(*val);
      }
      IdxIt++;
    }

    value = value * ivValue;

    break;
  }

  case Instruction::ExtractValue: {
    auto *EVI = dyn_cast<ExtractValueInst>(I);

    uint32_t evValue = 1;

    evValue = EVI->getNumIndices();

    // check element wise equality
    auto Idx = EVI->getIndices();
    const auto *IdxIt = Idx.begin();
    const auto *IdxEnd = Idx.end();

    while (IdxIt != IdxEnd) {
      auto *val = &*IdxIt;
      if (val) {
        evValue += reinterpret_cast<unsigned>(*val);
      }
      IdxIt++;
    }

    value = value * evValue;

    break;
  }

  case Instruction::Fence: {
    auto *FI = dyn_cast<FenceInst>(I);

    uint32_t fValue = 1;

    fValue = static_cast<unsigned>(FI->getOrdering());

    fValue += static_cast<unsigned>(FI->getSyncScopeID());

    value = value * fValue;

    break;
  }

  case Instruction::AtomicCmpXchg: {
    auto *AXI = dyn_cast<AtomicCmpXchgInst>(I);

    uint32_t axValue = 1;

    axValue = AXI->isVolatile() ? 4 : 40;
    axValue += AXI->isWeak() ? 5 : 50;
    axValue += static_cast<unsigned>(AXI->getSuccessOrdering());
    axValue += static_cast<unsigned>(AXI->getFailureOrdering());
    axValue += static_cast<unsigned>(AXI->getSyncScopeID());

    value = value * axValue;

    break;
  }

  case Instruction::AtomicRMW: {
    auto *ARI = dyn_cast<AtomicRMWInst>(I);

    uint32_t arValue = 1;

    arValue = static_cast<unsigned>(ARI->getOperation());
    arValue += ARI->isVolatile() ? 6 : 60;
    arValue += static_cast<unsigned>(ARI->getOrdering());
    arValue += static_cast<unsigned>(ARI->getSyncScopeID());

    value = value * arValue;
    break;
  }

  case Instruction::PHI: {
    if (Deterministic)
      value = pseudorand_value++;
    else
      value = std::rand() % 10000 + 100;
    break;
  }

  default:
    if (auto *CI = dyn_cast<CmpInst>(I)) {
      uint32_t cmpValue = 1;

      cmpValue = static_cast<unsigned>(CI->getPredicate()) + 1;

      value = value * cmpValue;
    }
  }

  // Return
  return value;
}

bool detectASM_fm(Function &F) {
  for (BasicBlock &B : F) {
    for (Instruction &I : B) {
      if (CallInst *callInst = dyn_cast<CallInst>(&I)) {
        if (callInst->isInlineAsm()) {
          return true;
        }
      }
    }
  }
  return false;
}

bool detect_bad_ndelay(Function &F) {
  for (BasicBlock &B : F) {
    for (Instruction &I : B) {
      if (CallInst *callInst = dyn_cast<CallInst>(&I)) {
        if (Function *calledFunction = callInst->getCalledFunction()) {
          if (calledFunction->getName() == "__bad_ndelay") {
            return true;
          }
        }
      }
    }
  }
  return false;
}

static bool checkAsmVolatility(Function &F) {
  for (BasicBlock &B : F) {
    for (Instruction &I : B) {
      if (CallInst *callInst = dyn_cast<CallInst>(&I)) {
        if (callInst->isInlineAsm()) {
          return true;
        }
      }
    }
  }
  return false;
}

static bool skipFunction(Function &F) {
  if (F.getName().equals("ftab_insert")) {
    return true;
  }
  return false;
}

bool ignoreFunction(Function &F) {
  if (skipFunction(F)) {
    return true;
  }
  if (checkAsmVolatility(F)) {
    return true;
  }
  if (detect_bad_ndelay(F)) {
    return true;
  }
  for (Instruction &I : instructions(F)) {
    if (auto *CB = dyn_cast<CallBase>(&I)) {
      if (Function *F2 = CB->getCalledFunction()) {
        if (auto ID = (Intrinsic::ID)F2->getIntrinsicID()) {
          if (Intrinsic::isOverloaded(ID))
            continue;
          if (Intrinsic::getName(ID).contains("permvar"))
            return true;
          if (Intrinsic::getName(ID).contains("vcvtps"))
            return true;
          if (Intrinsic::getName(ID).contains("avx"))
            return true;
          if (Intrinsic::getName(ID).contains("x86"))
            return true;
          if (Intrinsic::getName(ID).contains("arm"))
            return true;
        }
      }
    }
  }
  return false;
}

bool FunctionMerging::runImpl(
    Module &M, function_ref<TargetTransformInfo *(Function &)> GTTI) {

  StringSet<> AlwaysPreserved;
  AlwaysPreserved.insert("main");

  srand(time(nullptr)); // 设置随机种子

  FunctionMergingOptions Options =
      FunctionMergingOptions()
          .maximizeParameterScore(MaxParamScore)
          .matchOnlyIdenticalTypes(IdenticalType)
          .enableUnifiedReturnTypes(EnableUnifiedReturnType);

  FunctionMerger FM(&M);

  std::unique_ptr<Matcher<Function *>> matcher;

  int size = 0;
  for (auto &F : M) {
    if (F.isDeclaration() || F.isVarArg() ||
        (!HasWholeProgram && F.hasAvailableExternallyLinkage()))
      continue;
    size++;
  }

  // Create a threshold based on the application's size
  if (AdaptiveThreshold || AdaptiveBands) {
    double x = std::log10(size) / 10;
    RankingDistance = (double)(x - 0.3);
    if (RankingDistance < 0.05)
      RankingDistance = 0.05;
    if (RankingDistance > 0.4)
      RankingDistance = 0.4;

    if (AdaptiveBands) {
      float target_probability = 0.9;
      float offset = 0.1;
      unsigned tempBands = std::ceil(
          std::log(1.0 - target_probability) /
          std::log(1.0 - std::pow(RankingDistance + offset, LSHRows)));
      if (tempBands < LSHBands)
        LSHBands = tempBands;
    }
    if (AdaptiveThreshold)
      RankingDistance = 1 - RankingDistance;
    else
      RankingDistance = 1.0;
  }
  if (Debug) {
    errs() << "Threshold: " << RankingDistance << "\n";
    errs() << "LSHRows: " << LSHRows << "\n";
    errs() << "LSHBands: " << LSHBands << "\n";
  }

  if (!ToMergeFile.empty()) {
    matcher =
        std::make_unique<MatcherManual<Function *>>(FM, Options, ToMergeFile);
  } else if (EnableF3M) {
    matcher = std::make_unique<MatcherLSH<Function *>>(FM, Options, LSHRows,
                                                       LSHBands);
  } else {
    matcher = std::make_unique<MatcherFQ<Function *>>(FM, Options);
  }

  SearchStrategy strategy(LSHRows, LSHBands);
  for (auto &F : M) {
    if (F.isDeclaration() || F.isVarArg() ||
        (!HasWholeProgram && F.hasAvailableExternallyLinkage()))
      continue;
    if (ignoreFunction(F))
      continue;
    matcher->add_candidate(&F, EstimateFunctionSize(&F, GTTI(F)));
  }

  if (Debug) {
    errs() << "Number of Functions: " << matcher->size() << "\n";
    if (MatcherStats) {
      return false;
    }
  }

  unsigned TotalMerges = 0;

  while (matcher->size() > 0) {

    Function *F1 = matcher->next_candidate();
    auto &Rank = matcher->get_matches(F1);
    matcher->remove_candidate(F1);

    if (F1 == nullptr)
      continue;
    unsigned MergingTrialsCount = 0;

    std::string F1Name(GetValueName(F1));

    while (!Rank.empty()) {
      MatchInfo<Function *> match = Rank.back();
      Rank.pop_back();
      Function *F2 = match.candidate;

      if (F1 == nullptr || F2 == nullptr)
        continue;
      if (verifyFunction(*F1) || verifyFunction(*F2)) {
        errs() << "feisen:debug:verify function error\n";
        continue;
      }

      if (Debug) {
        errs() << "feisen:debug:F1: " << F1->getName()
               << " F2: " << F2->getName() << "\n";
        errs() << F1->getName().equals("") << "\n";
      }

      std::string F2Name(GetValueName(F2));
      MergingTrialsCount++;

      if (Debug)
        errs() << "Attempting: " << F1Name << ", " << F2Name << " : "
               << match.Distance << "\n";

      std::string Name = "_m_f_" + std::to_string(TotalMerges);
      FunctionMergeResult Result = FM.merge(F1, F2, Name, Options);
      if (Result.getMergedFunction() != nullptr && Result.Success) {
        match.Valid = !verifyFunction(*Result.getMergedFunction());
        if (!match.Valid) {
          Result.getMergedFunction()->eraseFromParent();
        } else {
          size_t MergedSize = EstimateFunctionSize(
              Result.getMergedFunction(), GTTI(*Result.getMergedFunction()));
          size_t Overhead = EstimateThunkOverhead(Result, AlwaysPreserved);

          size_t SizeF12 = MergedSize + Overhead;
          size_t SizeF1F2 = match.OtherSize + match.Size;

          match.MergedSize = SizeF12;
          match.Profitable = (SizeF12 + MergingOverheadThreshold) < SizeF1F2;

          if (!ToMergeFile.empty() || match.Profitable) {

            TotalMerges++;
            matcher->remove_candidate(F2);

            FM.updateCallGraph(Result, AlwaysPreserved, Options);

            // resolve phinode
            resolvePHI(*(Result.getMergedFunction()));

            if (ReuseMergedFunctions) {
              matcher->add_candidate(
                  Result.getMergedFunction(),
                  EstimateFunctionSize(Result.getMergedFunction(),
                                       GTTI(*Result.getMergedFunction())));
            }
            break;
          } 
            Result.getMergedFunction()->eraseFromParent();
          
        }
      }
      if (MergingTrialsCount >= ExplorationThreshold)
        break;
    }
  }

  return true;
}

PreservedAnalyses FunctionMergingPass::run(Module &M,
                                           ModuleAnalysisManager &AM) {
  FunctionMerging FM;
  if (Debug) {
    errs() << "ExplorationThreshold: " << ExplorationThreshold
           << "\n"; // feisen
    errs() << "RankingThreshold: " << RankingThreshold << "\n";
    errs() << "MergingOverheadThreshold" << MergingOverheadThreshold << "\n";
    errs() << "MaxParamScore: " << MaxParamScore << "\n";
    errs() << "IdenticalType: " << IdenticalType << "\n";
    errs() << "EnableUnifiedReturnType: " << EnableUnifiedReturnType << "\n";
    errs() << "EnableOperandReordering: " << EnableOperandReordering << "\n";
    errs() << "HasWholeProgram" << "HasWholeProgram" << "\n";
    errs() << "run \n"; // feisen
  }

  // skip some modules
  if (M.getName().find("/gold") != std::string::npos ||
      M.getName().find("/binutils") != std::string::npos ||
      M.getName().find("kernel-source") != std::string::npos ||
      (M.getName().find("arch/") != std::string::npos &&
       M.getName().str().compare(0, 5, "arch/") == 0) ||
      (M.getName().find("init/") != std::string::npos &&
       M.getName().str().compare(0, 5, "init/") == 0) ||
      (M.getName().find("kernel/") != std::string::npos &&
       M.getName().str().compare(0, 7, "kernel/") == 0)) {
    return PreservedAnalyses::all();
  }

  if (!FM.runImpl(M)) { //, GTTI))
    return PreservedAnalyses::all();
  }
  resolvePHI_fm(M, AM);
  return PreservedAnalyses::none();
}

static std::string GetValueName(const Value *V) {
  if (V) {
    std::string name;
    raw_string_ostream namestream(name);
    V->printAsOperand(namestream, false);
    return namestream.str();
  }
  return "[null]";
}

/// Create a cast instruction if needed to cast V to type DstType. We treat
/// pointer and integer types of the same bitwidth as equivalent, so this can be
/// used to cast them to each other where needed. The function returns the Value
/// itself if no cast is needed, or a new CastInst instance inserted before
/// InsertBefore. The integer type equivalent to pointers must be passed as
/// IntPtrType (get it from DataLayout). This is guaranteed to generate no-op
/// casts, otherwise it will assert.
// Value *FunctionMerger::createCastIfNeeded(Value *V, Type *DstType,
// IRBuilder<> &Builder, const FunctionMergingOptions &Options) {
Value *createCastIfNeeded(Value *V, Type *DstType, IRBuilder<> &Builder,
                          Type *IntPtrTy,
                          const FunctionMergingOptions &Options) {

  if (V->getType() == DstType || Options.IdenticalTypesOnly)
    return V;

  Value *Result;
  Type *OrigType = V->getType();

  if (OrigType->isStructTy()) {
    assert(DstType->isStructTy());
    assert(OrigType->getStructNumElements() == DstType->getStructNumElements());

    Result = UndefValue::get(DstType);
    for (unsigned int I = 0, E = OrigType->getStructNumElements(); I < E; ++I) {
      Value *ExtractedValue =
          Builder.CreateExtractValue(V, ArrayRef<unsigned int>(I));
      Value *Element =
          createCastIfNeeded(ExtractedValue, DstType->getStructElementType(I),
                             Builder, IntPtrTy, Options);
      Result =
          Builder.CreateInsertValue(Result, Element, ArrayRef<unsigned int>(I));
    }
    return Result;
  }
  assert(!DstType->isStructTy());

  if (OrigType->isPointerTy() &&
      (DstType->isIntegerTy() || DstType->isPointerTy())) {
    Result = Builder.CreatePointerCast(V, DstType, "merge_cast");
  } else if (OrigType->isIntegerTy() && DstType->isPointerTy() &&
             OrigType == IntPtrTy) {
    // Int -> Ptr
    Result = Builder.CreateCast(CastInst::IntToPtr, V, DstType, "merge_cast");
  } else {
    llvm_unreachable("Can only cast int -> ptr or ptr -> (ptr or int)");
  }

  return Result;
}

void FunctionMerger::CodeGenerator::removeRedundantInstructions(
    std::vector<Instruction *> &WorkInst, DominatorTree &DT) {
  std::set<Instruction *> SkipList;

  std::map<Instruction *, std::list<Instruction *>> UpdateList;

  for (Instruction *I1 : WorkInst) {
    if (SkipList.find(I1) != SkipList.end())
      continue;
    for (Instruction *I2 : WorkInst) {
      if (I1 == I2)
        continue;
      if (SkipList.find(I2) != SkipList.end())
        continue;
      assert(I1->getNumOperands() == I2->getNumOperands() &&
             "Should have the same num of operands!");
      bool AllEqual = true;
      for (unsigned i = 0; i < I1->getNumOperands(); ++i) {
        AllEqual = AllEqual && (I1->getOperand(i) == I2->getOperand(i));
      }

      if (AllEqual && DT.dominates(I1, I2)) {
        UpdateList[I1].push_back(I2);
        SkipList.insert(I2);
        SkipList.insert(I1);
      }
    }
  }

  for (auto &kv : UpdateList) {
    for (auto *I : kv.second) {
      erase(I);
      I->replaceAllUsesWith(kv.first);
      I->eraseFromParent();
    }
  }
}

////////////////////////////////////   SALSSA   ////////////////////////////////

static void postProcessFunction(Function &F) {
  legacy::FunctionPassManager FPM(F.getParent());

  FPM.add(createCFGSimplificationPass());
  FPM.doInitialization();
  FPM.run(F);
  FPM.doFinalization();
}

template <typename BlockListType>
static void CodeGen(BlockListType &Blocks1, BlockListType &Blocks2,
                    BasicBlock *EntryBB1, BasicBlock *EntryBB2,
                    Function *MergedFunc, Value *IsFunc1, BasicBlock *PreBB,
                    AlignedCode &AlignedSeq, ValueToValueMapTy &VMap,
                    std::unordered_map<BasicBlock *, BasicBlock *> &BlocksF1,
                    std::unordered_map<BasicBlock *, BasicBlock *> &BlocksF2,
                    std::unordered_map<Value *, BasicBlock *> &MaterialNodes) {

  auto CloneInst = [](IRBuilder<> &Builder, Function *MF,
                      Instruction *I) -> Instruction * {
    Instruction *NewI = nullptr;
    if (I->getOpcode() == Instruction::Ret) {
      if (MF->getReturnType()->isVoidTy()) {
        NewI = Builder.CreateRetVoid();
      } else {
        NewI = Builder.CreateRet(UndefValue::get(MF->getReturnType()));
      }
    } else {
      NewI = I->clone();
      for (unsigned i = 0; i < NewI->getNumOperands(); i++) {
        if (!isa<Constant>(I->getOperand(i)))
          NewI->setOperand(i, nullptr);
      }
      Builder.Insert(NewI);
    }

    SmallVector<std::pair<unsigned, MDNode *>, 8> MDs;
    NewI->getAllMetadata(MDs);
    for (std::pair<unsigned, MDNode *> MDPair : MDs) {
      NewI->setMetadata(MDPair.first, nullptr);
    }
    return NewI;
  };

  for (auto &Entry : AlignedSeq) {
    if (Entry.match()) {

      auto *I1 = dyn_cast<Instruction>(Entry.get(0));
      auto *I2 = dyn_cast<Instruction>(Entry.get(1));

      std::string BBName =
          (I1 == nullptr) ? "m.label.bb"
                          : (I1->isTerminator() ? "m.term.bb" : "m.inst.bb");

      BasicBlock *MergedBB =
          BasicBlock::Create(MergedFunc->getContext(), BBName, MergedFunc);

      MaterialNodes[Entry.get(0)] = MergedBB;
      MaterialNodes[Entry.get(1)] = MergedBB;

      if (I1 != nullptr && I2 != nullptr) {
        IRBuilder<> Builder(MergedBB);
        Instruction *NewI = CloneInst(Builder, MergedFunc, I1);

        VMap[I1] = NewI;
        VMap[I2] = NewI;
        BlocksF1[MergedBB] = I1->getParent();
        BlocksF2[MergedBB] = I2->getParent();
      } else {
        assert(isa<BasicBlock>(Entry.get(0)) && isa<BasicBlock>(Entry.get(1)) &&
               "Both nodes must be basic blocks!");
        auto *BB1 = dyn_cast<BasicBlock>(Entry.get(0));
        auto *BB2 = dyn_cast<BasicBlock>(Entry.get(1));

        VMap[BB1] = MergedBB;
        VMap[BB2] = MergedBB;
        BlocksF1[MergedBB] = BB1;
        BlocksF2[MergedBB] = BB2;

        // IMPORTANT: make sure any use in a blockaddress constant
        // operation is updated correctly
        for (User *U : BB1->users()) {
          if (auto *BA = dyn_cast<BlockAddress>(U)) {
            VMap[BA] = BlockAddress::get(MergedFunc, MergedBB);
          }
        }
        for (User *U : BB2->users()) {
          if (auto *BA = dyn_cast<BlockAddress>(U)) {
            VMap[BA] = BlockAddress::get(MergedFunc, MergedBB);
          }
        }

        IRBuilder<> Builder(MergedBB);
        for (Instruction &I : *BB1) {
          if (isa<PHINode>(&I)) {
            VMap[&I] = Builder.CreatePHI(I.getType(), 0);
          }
        }
        for (Instruction &I : *BB2) {
          if (isa<PHINode>(&I)) {
            VMap[&I] = Builder.CreatePHI(I.getType(), 0);
          }
        }
      }
    }
  }

  auto ChainBlocks = [](BasicBlock *SrcBB, BasicBlock *TargetBB,
                        Value *IsFunc1) {
    IRBuilder<> Builder(SrcBB);
    if (SrcBB->getTerminator() == nullptr) {
      Builder.CreateBr(TargetBB);
    } else {
      auto *Br = dyn_cast<BranchInst>(SrcBB->getTerminator());
      assert(Br && Br->isUnconditional() &&
             "Branch should be unconditional at this point!");
      BasicBlock *SuccBB = Br->getSuccessor(0);
      Br->eraseFromParent();
      Builder.CreateCondBr(IsFunc1, SuccBB, TargetBB);
    }
  };

  auto ProcessEachFunction_NonSeq =
      [&](int FuncIdx, std::unordered_map<BasicBlock *, BasicBlock *> &BlocksFX,
          Value *IsFunc1) {
        BasicBlock *LastMergedBB = nullptr;
        BasicBlock *NewBB = nullptr;

        for (auto &Entry : AlignedSeq) {
          Value *V = Entry.get(FuncIdx);
          if (V == nullptr)
            continue;

          if (BasicBlock *BB = dyn_cast<BasicBlock>(V)) {
            LastMergedBB = nullptr;
            NewBB = nullptr;
            if (auto It = MaterialNodes.find(BB); It != MaterialNodes.end()) {
              LastMergedBB = It->second;
            } else {
              std::string BBName = std::string("src.bb");
              NewBB = BasicBlock::Create(MergedFunc->getContext(), BBName,
                                         MergedFunc);
              VMap[BB] = NewBB;
              BlocksFX[NewBB] = BB;

              for (User *U : BB->users()) {
                if (auto *BA = dyn_cast<BlockAddress>(U)) {
                  VMap[BA] = BlockAddress::get(MergedFunc, NewBB);
                }
              }

              IRBuilder<> Builder(NewBB);
              for (Instruction &I : *BB) {
                if (isa<PHINode>(&I)) {
                  VMap[&I] = Builder.CreatePHI(I.getType(), 0);
                }
              }
            }
          } else if (Instruction *I = dyn_cast<Instruction>(V)) {
            if (isa<LandingPadInst>(I))
              continue;
            if (isa<PHINode>(I))
              continue;

            if (auto It = MaterialNodes.find(I); It != MaterialNodes.end()) {
              BasicBlock *NodeBB = It->second;
              if (LastMergedBB) {
                ChainBlocks(LastMergedBB, NodeBB, IsFunc1);
              } else {
                IRBuilder<> Builder(NewBB);
                Builder.CreateBr(NodeBB);
              }
              // end keep track
              LastMergedBB = NodeBB;
            } else {
              if (LastMergedBB) {
                std::string BBName = std::string("split.bb");
                NewBB = BasicBlock::Create(MergedFunc->getContext(), BBName,
                                           MergedFunc);
                ChainBlocks(LastMergedBB, NewBB, IsFunc1);
                BlocksFX[NewBB] = BB;
              }
              LastMergedBB = nullptr;

              IRBuilder<> Builder(NewBB);
              Instruction *NewI = CloneInst(Builder, MergedFunc, I);
              VMap[I] = NewI;
            }
          } else {
            errs() << "Should never get here!\n";
          }
        }
      };

  ProcessEachFunction_NonSeq(0, BlocksF1, IsFunc1);
  ProcessEachFunction_NonSeq(1, BlocksF2, IsFunc1);

  auto *BB1 = dyn_cast<BasicBlock>(VMap[EntryBB1]);
  auto *BB2 = dyn_cast<BasicBlock>(VMap[EntryBB2]);

  BlocksF1[PreBB] = BB1;
  BlocksF2[PreBB] = BB2;

  if (BB1 == BB2) {
    IRBuilder<> Builder(PreBB);
    Builder.CreateBr(BB1);
  } else {
    IRBuilder<> Builder(PreBB);
    Builder.CreateCondBr(IsFunc1, BB1, BB2);
  }
}

bool FunctionMerger::SALSSACodeGen::generate(
    AlignedCode &AlignedSeq, ValueToValueMapTy &VMap,
    const FunctionMergingOptions &Options) {

  LLVMContext &Context = CodeGenerator::getContext();
  Function *MergedFunc = CodeGenerator::getMergedFunction();
  Value *IsFunc1 = CodeGenerator::getFunctionIdentifier();
  Type *ReturnType = CodeGenerator::getMergedReturnType();
  bool RequiresUnifiedReturn = CodeGenerator::getRequiresUnifiedReturn();
  BasicBlock *EntryBB1 = CodeGenerator::getEntryBlock1();
  BasicBlock *EntryBB2 = CodeGenerator::getEntryBlock2();
  BasicBlock *PreBB = CodeGenerator::getPreBlock();

  Type *RetType1 = CodeGenerator::getReturnType1();
  Type *RetType2 = CodeGenerator::getReturnType2();

  Type *IntPtrTy = CodeGenerator::getIntPtrType();

  std::vector<BasicBlock *> &Blocks1 = CodeGenerator::getBlocks1();
  std::vector<BasicBlock *> &Blocks2 = CodeGenerator::getBlocks2();

  std::list<Instruction *> LinearOffendingInsts;
  std::set<Instruction *> OffendingInsts;
  std::map<Instruction *, std::map<Instruction *, unsigned>>
      CoalescingCandidates;

  std::vector<Instruction *> ListSelects;

  std::vector<AllocaInst *> Allocas;

  Value *RetUnifiedAddr = nullptr;
  Value *RetAddr1 = nullptr;
  Value *RetAddr2 = nullptr;

  // maps new basic blocks in the merged function to their original
  // correspondents
  std::unordered_map<BasicBlock *, BasicBlock *> BlocksF1;
  std::unordered_map<BasicBlock *, BasicBlock *> BlocksF2;
  std::unordered_map<Value *, BasicBlock *> MaterialNodes;

  CodeGen(Blocks1, Blocks2, EntryBB1, EntryBB2, MergedFunc, IsFunc1, PreBB,
          AlignedSeq, VMap, BlocksF1, BlocksF2, MaterialNodes);

  if (RequiresUnifiedReturn) {
    IRBuilder<> Builder(PreBB);
    RetUnifiedAddr = Builder.CreateAlloca(ReturnType);
    CodeGenerator::insert(dyn_cast<Instruction>(RetUnifiedAddr));

    RetAddr1 = Builder.CreateAlloca(RetType1);
    RetAddr2 = Builder.CreateAlloca(RetType2);
    CodeGenerator::insert(dyn_cast<Instruction>(RetAddr1));
    CodeGenerator::insert(dyn_cast<Instruction>(RetAddr2));
  }

  std::set<BranchInst *> XorBrConds;
  // assigning label operands

  for (auto &Entry : AlignedSeq) {
    Instruction *I1 = nullptr;
    Instruction *I2 = nullptr;

    if (Entry.get(0) != nullptr)
      I1 = dyn_cast<Instruction>(Entry.get(0));
    if (Entry.get(1) != nullptr)
      I2 = dyn_cast<Instruction>(Entry.get(1));

    // Skip non-instructions
    if (I1 == nullptr && I2 == nullptr)
      continue;

    if (Entry.match()) {

      Instruction *I = I1;
      if (I1->getOpcode() == Instruction::Ret) {
        I = (I1->getNumOperands() >= I2->getNumOperands()) ? I1 : I2;
      } else {
        assert(I1->getNumOperands() == I2->getNumOperands() &&
               "Num of Operands SHOULD be EQUAL\n");
      }

      auto *NewI = dyn_cast<Instruction>(VMap[I]);

      bool Handled = false;

      if (!Handled) {
        for (unsigned i = 0; i < I->getNumOperands(); i++) {

          Value *F1V = nullptr;
          Value *V1 = nullptr;
          if (i < I1->getNumOperands()) {
            F1V = I1->getOperand(i);
            V1 = MapValue(F1V, VMap);
            if (V1 == nullptr) {
              if (Debug)
                errs() << "ERROR: Null value mapped: V1 = "
                          "MapValue(I1->getOperand(i), "
                          "VMap);\n";
              return false;
            }
          } else {
            V1 = UndefValue::get(I2->getOperand(i)->getType());
          }

          Value *F2V = nullptr;
          Value *V2 = nullptr;
          if (i < I2->getNumOperands()) {
            F2V = I2->getOperand(i);
            V2 = MapValue(F2V, VMap);
            // assert(V2!=nullptr && "Mapped value should NOT be NULL!");

            if (V2 == nullptr) {
              if (Debug)
                errs() << "ERROR: Null value mapped: V2 = "
                          "MapValue(I2->getOperand(i), "
                          "VMap);\n";

              return false;
            }

          } else {
            V2 = UndefValue::get(I1->getOperand(i)->getType());
          }

          assert(V1 != nullptr && "Value should NOT be null!");
          assert(V2 != nullptr && "Value should NOT be null!");

          Value *V = V1; // first assume that V1==V2

          // handling just label operands for now
          if (!isa<BasicBlock>(V))
            continue;

          auto *F1BB = dyn_cast<BasicBlock>(F1V);
          auto *F2BB = dyn_cast<BasicBlock>(F2V);

          if (V1 != V2) {
            auto *BB1 = dyn_cast<BasicBlock>(V1);
            auto *BB2 = dyn_cast<BasicBlock>(V2);

            BasicBlock *SelectBB =
                BasicBlock::Create(Context, "bb.select", MergedFunc);
            IRBuilder<> BuilderBB(SelectBB);

            BlocksF1[SelectBB] = I1->getParent();
            BlocksF2[SelectBB] = I2->getParent();

            BuilderBB.CreateCondBr(IsFunc1, BB1, BB2);
            V = SelectBB;
          }

          if (F1BB->isLandingPad() || F2BB->isLandingPad()) {
            LandingPadInst *LP1 = F1BB->getLandingPadInst();
            LandingPadInst *LP2 = F2BB->getLandingPadInst();
            assert((LP1 != nullptr && LP2 != nullptr) &&
                   "Should be both as per the BasicBlock match!");
            (void)LP2;

            BasicBlock *LPadBB =
                BasicBlock::Create(Context, "lpad.bb", MergedFunc);
            IRBuilder<> BuilderBB(LPadBB);

            Instruction *NewLP = LP1->clone();
            BuilderBB.Insert(NewLP);

            BuilderBB.CreateBr(dyn_cast<BasicBlock>(V));

            BlocksF1[LPadBB] = I1->getParent();
            BlocksF2[LPadBB] = I2->getParent();

            VMap[F1BB->getLandingPadInst()] = NewLP;
            VMap[F2BB->getLandingPadInst()] = NewLP;

            V = LPadBB;
          }
          NewI->setOperand(i, V);
        }
      }

    } else { // if(entry.match())-else

      auto AssignLabelOperands =
          [&](Instruction *I,
              std::unordered_map<BasicBlock *, BasicBlock *> &BlocksReMap)
          -> bool {
        auto *NewI = dyn_cast<Instruction>(VMap[I]);
        for (unsigned i = 0; i < I->getNumOperands(); i++) {
          // handling just label operands for now
          if (!isa<BasicBlock>(I->getOperand(i)))
            continue;
          auto *FXBB = dyn_cast<BasicBlock>(I->getOperand(i));

          Value *V = MapValue(FXBB, VMap);
          if (V == nullptr)
            return false; // ErrorResponse;

          if (FXBB->isLandingPad()) {

            LandingPadInst *LP = FXBB->getLandingPadInst();
            assert(LP != nullptr && "Should have a landingpad inst!");

            BasicBlock *LPadBB =
                BasicBlock::Create(Context, "lpad.bb", MergedFunc);
            IRBuilder<> BuilderBB(LPadBB);

            Instruction *NewLP = LP->clone();
            BuilderBB.Insert(NewLP);
            VMap[LP] = NewLP;
            BlocksReMap[LPadBB] = I->getParent(); // FXBB;

            BuilderBB.CreateBr(dyn_cast<BasicBlock>(V));

            V = LPadBB;
          }

          NewI->setOperand(i, V);
        }
        return true;
      };

      if (I1 != nullptr && !AssignLabelOperands(I1, BlocksF1)) {
        if (Debug)
          errs() << "ERROR: Value should NOT be null\n";

        return false;
      }
      if (I2 != nullptr && !AssignLabelOperands(I2, BlocksF2)) {
        if (Debug)
          errs() << "ERROR: Value should NOT be null\n";

        return false;
      }
    }
  }

  auto MergeValues = [&](Value *V1, Value *V2,
                         Instruction *InsertPt) -> Value * {
    if (V1 == V2)
      return V1;

    if (V1 == ConstantInt::getTrue(Context) &&
        V2 == ConstantInt::getFalse(Context))
      return IsFunc1;

    if (V1 == ConstantInt::getFalse(Context) &&
        V2 == ConstantInt::getTrue(Context)) {
      IRBuilder<> Builder(InsertPt);
      return Builder.CreateNot(IsFunc1);
    }

    auto *IV1 = dyn_cast<Instruction>(V1);
    auto *IV2 = dyn_cast<Instruction>(V2);

    if (IV1 && IV2) {
      if (BlocksF2.find(IV1->getParent()) == BlocksF2.end() &&
          BlocksF1.find(IV2->getParent()) == BlocksF1.end()) {
        CoalescingCandidates[IV1][IV2]++;
        CoalescingCandidates[IV2][IV1]++;
      }
    }

    IRBuilder<> Builder(InsertPt);
    Instruction *Sel = (Instruction *)Builder.CreateSelect(IsFunc1, V1, V2);
    ListSelects.push_back(dyn_cast<Instruction>(Sel));
    return Sel;
  };

  auto AssignOperands = [&](Instruction *I, bool IsFuncId1) -> bool {
    auto *NewI = dyn_cast<Instruction>(VMap[I]);
    IRBuilder<> Builder(NewI);

    if (I->getOpcode() == Instruction::Ret && RequiresUnifiedReturn) {
      Value *V = MapValue(I->getOperand(0), VMap);
      if (V == nullptr) {
        return false; // ErrorResponse;
      }
      if (V->getType() != ReturnType) {
        Value *Addr = Builder.CreateAlloca(V->getType());
        Builder.CreateStore(V, Addr);
        Value *CastedAddr =
            Builder.CreatePointerCast(Addr, RetUnifiedAddr->getType());
        V = Builder.CreateLoad(ReturnType, CastedAddr);
      }
      NewI->setOperand(0, V);
    } else {
      for (unsigned i = 0; i < I->getNumOperands(); i++) {
        if (isa<BasicBlock>(I->getOperand(i)))
          continue;

        Value *V = MapValue(I->getOperand(i), VMap);
        if (V == nullptr) {
          return false; // ErrorResponse;
        }
        NewI->setOperand(i, V);
      }
    }

    return true;
  };

  for (auto &Entry : AlignedSeq) {
    Instruction *I1 = nullptr;
    Instruction *I2 = nullptr;

    if (Entry.get(0) != nullptr)
      I1 = dyn_cast<Instruction>(Entry.get(0));
    if (Entry.get(1) != nullptr)
      I2 = dyn_cast<Instruction>(Entry.get(1));

    if (I1 != nullptr && I2 != nullptr) {

      Instruction *I = I1;
      if (I1->getOpcode() == Instruction::Ret) {
        I = (I1->getNumOperands() >= I2->getNumOperands()) ? I1 : I2;
      } else {
        assert(I1->getNumOperands() == I2->getNumOperands() &&
               "Num of Operands SHOULD be EQUAL\n");
      }

      auto *NewI = dyn_cast<Instruction>(VMap[I]);

      IRBuilder<> Builder(NewI);

      if (EnableOperandReordering && isa<BinaryOperator>(NewI) &&
          I->isCommutative()) {

        auto *BO1 = dyn_cast<BinaryOperator>(I1);
        auto *BO2 = dyn_cast<BinaryOperator>(I2);
        Value *VL1 = MapValue(BO1->getOperand(0), VMap);
        Value *VL2 = MapValue(BO2->getOperand(0), VMap);
        Value *VR1 = MapValue(BO1->getOperand(1), VMap);
        Value *VR2 = MapValue(BO2->getOperand(1), VMap);
        if (VL1 == VR2 && VL2 != VR2) {
          std::swap(VL2, VR2);
          // CountOpReorder++;
        } else if (VL2 == VR1 && VL1 != VR1) {
          std::swap(VL1, VR1);
        }

        std::vector<std::pair<Value *, Value *>> Vs;
        Vs.emplace_back(VL1, VL2);
        Vs.emplace_back(VR1, VR2);

        for (unsigned i = 0; i < Vs.size(); i++) {
          Value *V1 = Vs[i].first;
          Value *V2 = Vs[i].second;

          Value *V = MergeValues(V1, V2, NewI);
          if (V == nullptr) {
            if (Debug) {
              errs() << "Could Not select:\n";
              errs() << "ERROR: Value should NOT be null\n";
            }
            // MergedFunc->eraseFromParent();
            return false; // ErrorResponse;
          }

          Value *CastedV = createCastIfNeeded(V, NewI->getOperand(i)->getType(),
                                              Builder, IntPtrTy);
          NewI->setOperand(i, CastedV);
        }
      } else {
        for (unsigned i = 0; i < I->getNumOperands(); i++) {
          if (isa<BasicBlock>(I->getOperand(i)))
            continue;

          Value *V1 = nullptr;
          if (i < I1->getNumOperands()) {
            V1 = MapValue(I1->getOperand(i), VMap);
            if (V1 == nullptr) {
              if (Debug)
                errs() << "ERROR: Null value mapped: V1 = "
                          "MapValue(I1->getOperand(i), "
                          "VMap);\n";
              return false;
            }
          } else {
            V1 = UndefValue::get(I2->getOperand(i)->getType());
          }

          Value *V2 = nullptr;
          if (i < I2->getNumOperands()) {
            V2 = MapValue(I2->getOperand(i), VMap);

            if (V2 == nullptr) {
              if (Debug)
                errs() << "ERROR: Null value mapped: V2 = "
                          "MapValue(I2->getOperand(i), "
                          "VMap);\n";

              return false;
            }

          } else {
            V2 = UndefValue::get(I1->getOperand(i)->getType());
          }

          assert(V1 != nullptr && "Value should NOT be null!");
          assert(V2 != nullptr && "Value should NOT be null!");

          Value *V = MergeValues(V1, V2, NewI);
          if (V == nullptr) {
            if (Debug) {
              errs() << "Could Not select:\n";
              errs() << "ERROR: Value should NOT be null\n";
            }

            return false; // ErrorResponse;
          }

          NewI->setOperand(i, V);

        } // end for operands
      }
    } // end if isomorphic
    else {
      if (I1 != nullptr && !AssignOperands(I1, true)) {
        if (Debug)
          errs() << "ERROR: Value should NOT be null\n";

        return false;
      }
      if (I2 != nullptr && !AssignOperands(I2, false)) {
        if (Debug)
          errs() << "ERROR: Value should NOT be null\n";

        return false;
      }
    } // end 'if-else' non-isomorphic

  } // end for nodes
  if (Debug)
    errs() << "NumSelects: " << ListSelects.size() << "\n";
  if (ListSelects.size() > MaxNumSelection) {
    if (Debug)
      errs() << "Bailing out: Operand selection threshold\n";

    return false;
  }

  auto AssignPHIOperandsInBlock =
      [&](BasicBlock *BB,
          std::unordered_map<BasicBlock *, BasicBlock *> &BlocksReMap) -> bool {
    for (Instruction &I : *BB) {
      if (auto *PHI = dyn_cast<PHINode>(&I)) {
        auto *NewPHI = dyn_cast<PHINode>(VMap[PHI]);

        std::set<int> FoundIndices;

        for (auto It = pred_begin(NewPHI->getParent()),
                  E = pred_end(NewPHI->getParent());
             It != E; It++) {

          BasicBlock *NewPredBB = *It;

          Value *V = nullptr;

          if (BlocksReMap.find(NewPredBB) != BlocksReMap.end()) {
            int Index = PHI->getBasicBlockIndex(BlocksReMap[NewPredBB]);
            if (Index >= 0) {
              V = MapValue(PHI->getIncomingValue(Index), VMap);
              FoundIndices.insert(Index);
            }
          }

          if (V == nullptr) {
            V = UndefValue::get(NewPHI->getType());
          }

          NewPHI->addIncoming(V, NewPredBB);
        }
        if (FoundIndices.size() != PHI->getNumIncomingValues()) {
          if (Debug) {
            PHI->print(errs());
            errs() << "\n";

            errs() << "feisen: ";
            errs() << "FoundIndices.size(): " << FoundIndices.size() << " \n";
            errs() << "PHI->getNumIncomingValues()"
                   << PHI->getNumIncomingValues() << " \n";
          }
          return false;
        }
      }
    }
    return true;
  };

  for (BasicBlock *BB1 : Blocks1) {
    if (!AssignPHIOperandsInBlock(BB1, BlocksF1)) {
      if (Debug)
        errs() << "ERROR: PHI assignment\n";

      return false;
    }
  }
  for (BasicBlock *BB2 : Blocks2) {
    if (!AssignPHIOperandsInBlock(BB2, BlocksF2)) {
      if (Debug)
        errs() << "ERROR: PHI assignment\n";
      return false;
    }
  }

  // Replace select statements by merged PHIs

  // Collect candidate pairs of PHI Nodes
  SmallSet<std::pair<PHINode *, PHINode *>, 16> CandPHI;
  for (Instruction *I : ListSelects) {
    SelectInst *SI = dyn_cast<SelectInst>(I);
    assert(SI != nullptr);

    PHINode *PT = dyn_cast<PHINode>(SI->getTrueValue());
    PHINode *PF = dyn_cast<PHINode>(SI->getFalseValue());

    if (PT == nullptr || PF == nullptr)
      continue;

    // Only pair PHI Nodes in the same block
    if (PT->getParent() != PF->getParent())
      continue;

    CandPHI.insert({PT, PF});
  }

  SmallSet<PHINode *, 8> RemovedPHIs;
  for (auto [PT, PF] : CandPHI) {
    if ((RemovedPHIs.count(PT) > 0) || (RemovedPHIs.count(PF) > 0))
      continue;
    // Merge PT and PF if:
    // 1) their defined incoming values do not overlap
    // 2) their uses are only select statements on IsFunc1
    bool valid = true;
    SmallVector<SelectInst *> CandSel;

    // Are PHIs mergeable?
    for (unsigned i = 0; i < PT->getNumIncomingValues() && valid; ++i) {
      // if PT incoming value is Undef, this edge pair is mergeable
      Value *VT = PT->getIncomingValue(i);
      if (dyn_cast<UndefValue>(VT) != nullptr)
        continue;

      // if the PF incoming value for the same block is Undef,
      // this edge pair is mergeable
      BasicBlock *PredBB = PT->getIncomingBlock(i);
      if (PF->getBasicBlockIndex(PredBB) < 0) {
        errs() << "PHI ERROR\n";
      }
      Value *VF = PF->getIncomingValueForBlock(PredBB);
      if (dyn_cast<UndefValue>(VF) != nullptr)
        continue;

      // If the two incoming values are the same, then we can merge them
      if (VT == VF)
        continue;

      valid = false;
    }

    if (!valid)
      continue;

    // Are PHIs only used together in select statements?
    for (auto *UI : PT->users()) {
      SelectInst *SI = dyn_cast<SelectInst>(UI);
      if (SI == nullptr) {
        valid = false;
        break;
      }

      if ((SI->getTrueValue() != PT) || (SI->getFalseValue() != PF)) {
        valid = false;
        break;
      }

      if (SI->getCondition() != IsFunc1) {
        valid = false;
        break;
      }
      CandSel.push_back(SI);
    }

    if (!valid)
      continue;

    // Do the actual PHI merging using PT
    for (unsigned i = 0; i < PT->getNumIncomingValues() && valid; ++i) {
      // If edge is set, use it
      if (dyn_cast<UndefValue>(PT->getIncomingValue(i)) == nullptr)
        continue;

      // If edge not set, copy it from PF
      BasicBlock *PredBB = PT->getIncomingBlock(i);
      PT->setIncomingValue(i, PF->getIncomingValueForBlock(PredBB));
    }

    PF->replaceAllUsesWith(PT);
    PF->eraseFromParent();
    RemovedPHIs.insert(PF);

    // Replace all uses of the select statements with PT
    for (SelectInst *SI : CandSel) {
      SI->replaceAllUsesWith(PT);
      SI->eraseFromParent();
    }
  }

  DominatorTree DT(*MergedFunc);

  for (Instruction &I : instructions(MergedFunc)) {
    if (auto *PHI = dyn_cast<PHINode>(&I)) {
      for (unsigned i = 0; i < PHI->getNumIncomingValues(); i++) {
        BasicBlock *BB = PHI->getIncomingBlock(i);
        if (BB == nullptr)
          errs() << "Null incoming block\n";
        Value *V = PHI->getIncomingValue(i);
        if (V == nullptr)
          errs() << "Null incoming value\n";
        if (auto *IV = dyn_cast<Instruction>(V)) {
          if (BB->getTerminator() == nullptr) {
            if (Debug)
              errs() << "ERROR: Null terminator\n";
            return false;
          }
          if (!DT.dominates(IV, BB->getTerminator())) {
            if (OffendingInsts.count(IV) == 0) {
              OffendingInsts.insert(IV);
              LinearOffendingInsts.push_back(IV);
            }
          }
        }
      }
    } else {
      for (unsigned i = 0; i < I.getNumOperands(); i++) {
        if (I.getOperand(i) == nullptr) {
          if (Debug)
            errs() << "ERROR: Null operand\n";
          return false;
        }
        if (auto *IV = dyn_cast<Instruction>(I.getOperand(i))) {
          if (!DT.dominates(IV, &I)) {
            if (OffendingInsts.count(IV) == 0) {
              OffendingInsts.insert(IV);
              LinearOffendingInsts.push_back(IV);
            }
          }
        }
      }
    }
  }

  for (BranchInst *NewBr : XorBrConds) {
    IRBuilder<> Builder(NewBr);
    Value *XorCond = Builder.CreateXor(NewBr->getCondition(), IsFunc1);
    NewBr->setCondition(XorCond);
  }

  auto StoreInstIntoAddr = [](Instruction *IV, Value *Addr) {
    IRBuilder<> Builder(IV->getParent());
    if (IV->isTerminator()) {
      BasicBlock *SrcBB = IV->getParent();
      if (auto *II = dyn_cast<InvokeInst>(IV)) {
        BasicBlock *DestBB = II->getNormalDest();

        Builder.SetInsertPoint(&*DestBB->getFirstInsertionPt());
        // create PHI
        PHINode *PHI = Builder.CreatePHI(IV->getType(), 0);
        for (auto PredIt = pred_begin(DestBB), PredE = pred_end(DestBB);
             PredIt != PredE; PredIt++) {
          BasicBlock *PredBB = *PredIt;
          if (PredBB == SrcBB) {
            PHI->addIncoming(IV, PredBB);
          } else {
            PHI->addIncoming(UndefValue::get(IV->getType()), PredBB);
          }
        }
        Builder.CreateStore(PHI, Addr);
      } else {
        for (auto SuccIt = succ_begin(SrcBB), SuccE = succ_end(SrcBB);
             SuccIt != SuccE; SuccIt++) {
          BasicBlock *DestBB = *SuccIt;

          Builder.SetInsertPoint(&*DestBB->getFirstInsertionPt());
          // create PHI
          PHINode *PHI = Builder.CreatePHI(IV->getType(), 0);
          for (auto PredIt = pred_begin(DestBB), PredE = pred_end(DestBB);
               PredIt != PredE; PredIt++) {
            BasicBlock *PredBB = *PredIt;
            if (PredBB == SrcBB) {
              PHI->addIncoming(IV, PredBB);
            } else {
              PHI->addIncoming(UndefValue::get(IV->getType()), PredBB);
            }
          }
          Builder.CreateStore(PHI, Addr);
        }
      }
    } else {
      Instruction *LastI = nullptr;
      Instruction *InsertPt = nullptr;
      for (Instruction &I : *IV->getParent()) {
        InsertPt = &I;
        if (LastI == IV)
          break;
        LastI = &I;
      }
      if (isa<PHINode>(InsertPt) || isa<LandingPadInst>(InsertPt)) {
        Builder.SetInsertPoint(&*IV->getParent()->getFirstInsertionPt());
      } else
        Builder.SetInsertPoint(InsertPt);

      Builder.CreateStore(IV, Addr);
    }
  };

  auto MemfyInst = [&](std::set<Instruction *> &InstSet) -> AllocaInst * {
    if (InstSet.empty())
      return nullptr;
    IRBuilder<> Builder(&*PreBB->getFirstInsertionPt());
    AllocaInst *Addr = Builder.CreateAlloca((*InstSet.begin())->getType());
    Type *Ty = Addr->getAllocatedType();

    for (Instruction *I : InstSet) {
      for (auto UIt = I->use_begin(), E = I->use_end(); UIt != E;) {
        Use &UI = *UIt;
        UIt++;

        auto *User = cast<Instruction>(UI.getUser());

        if (auto *PHI = dyn_cast<PHINode>(User)) {
          auto InsertionPt =
              PHI->getIncomingBlock(UI.getOperandNo())->getTerminator();
          if (InsertionPt == I)
            continue;
          IRBuilder<> Builder(InsertionPt);
          UI.set(Builder.CreateLoad(Ty, Addr));
        } else {
          IRBuilder<> Builder(User);
          UI.set(Builder.CreateLoad(Ty, Addr));
        }
      }
    }

    for (Instruction *I : InstSet)
      StoreInstIntoAddr(I, Addr);

    return Addr;
  };

  auto isCoalescingProfitable = [&](Instruction *I1, Instruction *I2) -> bool {
    std::set<BasicBlock *> BBSet1;
    std::set<BasicBlock *> UnionBB;
    for (User *U : I1->users()) {
      if (auto *UI = dyn_cast<Instruction>(U)) {
        BasicBlock *BB1 = UI->getParent();
        BBSet1.insert(BB1);
        UnionBB.insert(BB1);
      }
    }

    unsigned Intersection = 0;
    for (User *U : I2->users()) {
      if (auto *UI = dyn_cast<Instruction>(U)) {
        BasicBlock *BB2 = UI->getParent();
        UnionBB.insert(BB2);
        if (BBSet1.find(BB2) != BBSet1.end())
          Intersection++;
      }
    }

    const float Threshold = 0.7;
    return (float(Intersection) / float(UnionBB.size()) > Threshold);
  };

  auto OptimizeCoalescing =
      [&](Instruction *I, std::set<Instruction *> &InstSet,
          std::map<Instruction *, std::map<Instruction *, unsigned>>
              &CoalescingCandidates,
          std::set<Instruction *> &Visited) {
        Instruction *OtherI = nullptr;
        unsigned Score = 0;
        if (CoalescingCandidates.find(I) != CoalescingCandidates.end()) {
          for (auto &Pair : CoalescingCandidates[I]) {
            if (Pair.second > Score &&
                Visited.find(Pair.first) == Visited.end()) {
              if (isCoalescingProfitable(I, Pair.first)) {
                OtherI = Pair.first;
                Score = Pair.second;
              }
            }
          }
        }
        if (OtherI) {
          InstSet.insert(OtherI);
        }
      };

  if (MergedFunc != nullptr) {
    if (((float)OffendingInsts.size()) / ((float)AlignedSeq.size()) > 4.5) {
      if (Debug)
        errs() << "Bailing out\n";

      return false;
    }
    std::set<Instruction *> Visited;
    for (Instruction *I : LinearOffendingInsts) {
      if (Visited.find(I) != Visited.end())
        continue;

      std::set<Instruction *> InstSet;
      InstSet.insert(I);

      // Create a coalescing group in InstSet
      if (EnableSALSSACoalescing)
        OptimizeCoalescing(I, InstSet, CoalescingCandidates, Visited);

      for (Instruction *OtherI : InstSet)
        Visited.insert(OtherI);

      AllocaInst *Addr = MemfyInst(InstSet);
      if (Addr)
        Allocas.push_back(Addr);
    }

    DominatorTree DT(*MergedFunc);
    PromoteMemToReg(Allocas, DT, nullptr);

    if (verifyFunction(*MergedFunc)) {

      return false;
    }

    postProcessFunction(*MergedFunc);
  }

  return MergedFunc != nullptr;
}
