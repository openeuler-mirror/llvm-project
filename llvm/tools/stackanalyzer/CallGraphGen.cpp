//===--- CallGraphGen.cpp - Analyze the callgraph of a LLVM bitcode file using
// pointer analysis ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "CallGraphGen.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/Instructions.h"

using namespace llvm;

std::unique_ptr<ConstraintGraphNode>
ConstraintGraph::createInitialConstraintNode() {
  UndefValue *UndefValue = UndefValue::get(Type::getVoidTy(M.getContext()));
  return std::make_unique<ConstraintGraphNode>(UndefValue, this);
}

ConstraintGraph::ConstraintGraph(Module &Module) : M(Module) {
  InitialConstraintNode = createInitialConstraintNode().get();
  for (auto &F : M) {
    if (F.hasExternalLinkage())
      continue;
    if (F.isIntrinsic())
      continue;
  }
}

ConstraintGraphNode *ConstraintGraph::getOrInsertConstraintNode(Value *V) {
  auto &CSN = ConstraintGraphNodes[V];
  if (CSN) {
    return CSN.get();
  }
  CSN = std::make_unique<ConstraintGraphNode>(V, this);
  return CSN.get();
}

Constraint *ConstraintGraph::getOrInsertConstraint(Value *Dst, Value *Src,
                                                   ConstraintKind Kind) {
  for (auto &C : Constraints) {
    if (C->Src->V == Src && C->Dst->V == Dst && C->Kind == Kind) {
      return C.get();
    }
  }
  auto *SrcNode = getOrInsertConstraintNode(Src);
  auto *DstNode = getOrInsertConstraintNode(Dst);
  auto ConstraitEdge = std::make_unique<Constraint>(DstNode, SrcNode, Kind);
  SrcNode->addConstraint(DstNode, Kind);
  Constraint *ConstraitEdgePtr = ConstraitEdge.get();
  Constraints.push_back(std::move(ConstraitEdge));
  return ConstraitEdgePtr;
}

Constraint *ConstraintGraph::insertConstraint(Value *Dst, Value *Src,
                                              ConstraintKind Kind) {
  auto *SrcNode = getOrInsertConstraintNode(Src);
  auto *DstNode = getOrInsertConstraintNode(Dst);
  auto ConstraitEdge = std::make_unique<Constraint>(DstNode, SrcNode, Kind);
  SrcNode->addConstraint(DstNode, Kind);
  Constraint *ConstraitEdgePtr = ConstraitEdge.get();
  Constraints.push_back(std::move(ConstraitEdge));
  return ConstraitEdgePtr;
}

AnalysisKey PACallGraphAnalysis::Key;

PACallGraphAnalysis::Result PACallGraphAnalysis::run(Module &M,
                                                     ModuleAnalysisManager &) {
  auto FI = M.begin(), FE = M.end();
  for (; FI != FE; ++FI) {
    if (FI->getName() == Config.EntryFunction) {
      break;
    }
  }
  DataflowResult<PAAnalysisDataflowFacts>::Type ResultFact;
  PAAnalysisDataflowFacts InitFact;
  PointerAnalysisVisitor PAVisitor(M);
  if (Config.UseAnders) {
    compForwardDataflow(&(*FI), &PAVisitor, &ResultFact, InitFact);
    PAVisitor.solveConstraint();
  }
  PAVisitor.removeRedundantCallEdge();
  if (Config.UseDebug) {
    PAVisitor.printConstraintGraph(outs());
    PAVisitor.printPointToSetMap(outs());
  }
  return std::move(PAVisitor.CG);
}

// for debugging purpose, reference:
// https://github.com/SunnyWadkar/LLVM-DataFlow-Analysis/blob/master/Dataflow/available-support.cpp
static std::string getShortValueName(const Value *V) {
  if (auto *Arg = dyn_cast<Argument>(V)) {
    std::string S = "";
    raw_string_ostream *Strm = new raw_string_ostream(S);
    Arg->print(*Strm);
    std::string ArgName = Strm->str();
    size_t Idx = ArgName.find(" ");
    return ArgName.substr(Idx) + ":" + Arg->getParent()->getName().str();
  }
  if (auto *InstV = dyn_cast<Instruction>(V)) {
    std::string S = "";
    raw_string_ostream *Strm = new raw_string_ostream(S);
    V->print(*Strm);
    std::string Inst = Strm->str();
    size_t Idx1 = Inst.find("%");
    size_t Idx2 = Inst.find(" ", Idx1);
    if (Idx1 != std::string::npos && Idx2 != std::string::npos) {
      return Inst.substr(Idx1, Idx2 - Idx1) + ":" +
             InstV->getFunction()->getName().str();
    }
    return "\"" + Inst + "\"";
  }
  if (const ConstantInt *Cint = dyn_cast<ConstantInt>(V)) {
    std::string S = "";
    raw_string_ostream *Strm = new raw_string_ostream(S);
    Cint->getValue().print(*Strm, true);
    return Strm->str();
  }
  if (V->getName().str().length() > 0) {
    return V->getName().str();
  }
  std::string S = "";
  raw_string_ostream *Strm = new raw_string_ostream(S);
  V->print(*Strm);
  std::string Inst = Strm->str();
  return "\"" + Inst + "\"";
}

PointerAnalysisVisitor::PointerAnalysisVisitor(Module &InitModule)
    : CSG(ConstraintGraph(InitModule)), CG(InitModule) {}

void PointerAnalysisVisitor::merge(PAAnalysisDataflowFacts *Facts,
                                   const PAAnalysisDataflowFacts &OtherFacts) {
  Facts->insert(Facts->end(), OtherFacts.begin(), OtherFacts.end());
  std::sort(Facts->begin(), Facts->end());
  auto Last = std::unique(Facts->begin(), Facts->end());
  Facts->erase(Last, Facts->end());
}

void PointerAnalysisVisitor::compDFVal(Instruction *Inst,
                                       PAAnalysisDataflowFacts *Dfval) {
  switch (Inst->getOpcode()) {
  case Instruction::Load: {
    transfer(dyn_cast<LoadInst>(Inst), Dfval);
    break;
  }
  case Instruction::Store: {
    transfer(dyn_cast<StoreInst>(Inst), Dfval);
    break;
  }
  case Instruction::Call: {
    transfer(dyn_cast<CallInst>(Inst), Dfval);
    break;
  }
  case Instruction::GetElementPtr: {
    transfer(dyn_cast<GetElementPtrInst>(Inst), Dfval);
    break;
  }
  case Instruction::Ret: {
    transfer(dyn_cast<ReturnInst>(Inst), Dfval);
    break;
  }
  case Instruction::PHI: {
    transfer(dyn_cast<PHINode>(Inst), Dfval);
    break;
  }
  case Instruction::BitCast: {
    transfer(dyn_cast<BitCastInst>(Inst), Dfval);
    break;
  }
  case Instruction::Select: {
    transfer(dyn_cast<SelectInst>(Inst), Dfval);
    break;
  }
  case Instruction::IntToPtr: {
    transfer(dyn_cast<IntToPtrInst>(Inst), Dfval);
    break;
  }
  }
}

void PointerAnalysisVisitor::transfer(LoadInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *Addr = Inst->getPointerOperand();
  auto *Constraint =
      CSG.getOrInsertConstraint(Inst, Addr, ConstraintKind::Load);
  Dfval->push_back(Constraint);
}

void PointerAnalysisVisitor::transfer(StoreInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *Addr = Inst->getPointerOperand();
  auto *Val = Inst->getValueOperand();
  Constraint *Cstrt;
  if (isa<Function>(Val)) {
    Cstrt = CSG.getOrInsertConstraint(Addr, Val, ConstraintKind::GetAddr);
  } else {
    Cstrt = CSG.getOrInsertConstraint(Addr, Val, ConstraintKind::Store);
  }
  Dfval->push_back(Cstrt);
}

void PointerAnalysisVisitor::transfer(GetElementPtrInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *StructVal = Inst->getPointerOperand();
  auto *ConstraintFrom =
      CSG.getOrInsertConstraint(Inst, StructVal, ConstraintKind::Subset);
  auto *ConstraintTo =
      CSG.getOrInsertConstraint(StructVal, Inst, ConstraintKind::Subset);
  Dfval->push_back(ConstraintFrom);
  Dfval->push_back(ConstraintTo);
}

void PointerAnalysisVisitor::transfer(PHINode *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  for (unsigned I = 0, NumOperands = Inst->getNumIncomingValues();
       I != NumOperands; ++I) {
    auto *PHIArg = Inst->getIncomingValue(I);
    auto *Constraint =
        CSG.getOrInsertConstraint(Inst, PHIArg, ConstraintKind::Subset);
    Dfval->push_back(Constraint);
  }
  if (Inst->getType()->isPointerTy()) {
    for (unsigned I = 0, NumOperands = Inst->getNumIncomingValues();
         I != NumOperands; ++I) {
      auto *PHIArg = Inst->getIncomingValue(I);
      auto *ConstraintTo =
          CSG.getOrInsertConstraint(PHIArg, Inst, ConstraintKind::Subset);
      Dfval->push_back(ConstraintTo);
    }
  }
}

void PointerAnalysisVisitor::transfer(BitCastInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *Src = Inst->getOperand(0);
  auto *Constraint =
      CSG.getOrInsertConstraint(Inst, Src, ConstraintKind::Subset);
  Dfval->push_back(Constraint);
  if (Src->getType()->isPointerTy()) {
    auto *ConstraintTo =
        CSG.getOrInsertConstraint(Src, Inst, ConstraintKind::Subset);
    Dfval->push_back(ConstraintTo);
  }
}

void PointerAnalysisVisitor::transfer(SelectInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *TrueVal = Inst->getTrueValue();
  auto *FalseVal = Inst->getFalseValue();
  auto *ConstraintTrue =
      CSG.getOrInsertConstraint(Inst, TrueVal, ConstraintKind::Subset);

  auto *ConstraintFalse =
      CSG.getOrInsertConstraint(Inst, FalseVal, ConstraintKind::Subset);

  Dfval->push_back(ConstraintTrue);
  Dfval->push_back(ConstraintFalse);
  if (TrueVal->getType()->isPointerTy()) {
    auto *ConstraintTo =
        CSG.getOrInsertConstraint(TrueVal, Inst, ConstraintKind::Subset);
    Dfval->push_back(ConstraintTo);
  }
  if (FalseVal->getType()->isPointerTy()) {
    auto *ConstraintTo =
        CSG.getOrInsertConstraint(FalseVal, Inst, ConstraintKind::Subset);
    Dfval->push_back(ConstraintTo);
  }
}

void PointerAnalysisVisitor::transfer(IntToPtrInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *Src = Inst->getOperand(0);
  auto *Constraint =
      CSG.getOrInsertConstraint(Inst, Src, ConstraintKind::Subset);
  Dfval->push_back(Constraint);
}

void PointerAnalysisVisitor::transfer(CallInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *Callee = Inst->getCalledFunction();
  if (!Callee) {
    auto *CalleeValue = Inst->getCalledOperand();
    auto *Constraint =
        CSG.getOrInsertConstraint(Inst, CalleeValue, ConstraintKind::Unsolved);
    for (unsigned I = 0, NumOperands = Inst->arg_size(); I != NumOperands;
         ++I) {
      auto *RArg = Inst->getArgOperand(I);
      auto *CSGN = CSG.getOrInsertConstraintNode(RArg);
      UnresolvedArgs[Constraint->Src].push_back(CSGN);
    }
    Dfval->push_back(Constraint);
    ConstraintFunctionMap[Constraint] = CurrentFunction;
  } else {
    auto *PrevFunction = CurrentFunction;
    if (Callee->isIntrinsic() || Callee->isDeclaration())
      return;
    for (unsigned I = 0, NumOperands = Inst->arg_size(); I != NumOperands;
         ++I) {
      auto *RArg = Inst->getArgOperand(I);
      auto *FArg = Callee->getArg(I);
      Dfval->push_back(
          CSG.getOrInsertConstraint(FArg, RArg, ConstraintKind::Subset));
      if (RArg->getType()->isPointerTy()) {
        Dfval->push_back(
            CSG.getOrInsertConstraint(RArg, FArg, ConstraintKind::Subset));
      }
    }
    DataflowResult<PAAnalysisDataflowFacts>::Type SubroutineResult;
    PAAnalysisDataflowFacts SubroutineInitFact;
    compForwardDataflow(Callee, this, &SubroutineResult, SubroutineInitFact);
    for (auto *V : FunctionReturnValueMap[Callee]) {
      Dfval->push_back(
          CSG.getOrInsertConstraint(Inst, V, ConstraintKind::Subset));
    }
    CurrentFunction = PrevFunction;
  }
}

void PointerAnalysisVisitor::transfer(ReturnInst *Inst,
                                      PAAnalysisDataflowFacts *Dfval) {
  auto *RetVal = Inst->getReturnValue();
  if (!RetVal)
    return;
  FunctionReturnValueMap[CurrentFunction].insert(RetVal);
}

using ConstraintSolverFn = void (PointerAnalysisVisitor::*)(
    const Constraint *Cstrt, ConstraintGraphNode *CSNode);

static ConstraintSolverFn ConstraintSolvers[] = {
    &PointerAnalysisVisitor::solveSubsetConstraint,
    &PointerAnalysisVisitor::solveGetAddrConstraint,
    &PointerAnalysisVisitor::solveLoadConstraint,
    &PointerAnalysisVisitor::solveStoreConstraint,
    &PointerAnalysisVisitor::solveUnsolvedConstraint,
    nullptr // ConstraintKind::Init
};

void PointerAnalysisVisitor::solveConstraint() {
  for (auto &CSNode : CSG) {
    Worklist.push_back(CSNode.second.get());
  }
  while (!Worklist.empty()) {
    auto *CSNode = Worklist.front();
    Worklist.pop_front();
    auto CurrentConstraints = CSG.getConstraints();
    for (auto *Constraint : CSG.getConstraints()) {
      if (Constraint->Src == CSNode) {
        (this->*ConstraintSolvers[static_cast<unsigned>(Constraint->Kind)])(
            Constraint, CSNode);
      }
    }
  }
}

void PointerAnalysisVisitor::solveLoadConstraint(const Constraint *Cstrt,
                                                 ConstraintGraphNode *CSNode) {
  if (PointToSetMap[Cstrt->Src].empty() &&
      !CSG.hasConstraintEdge(Cstrt->Src->V, Cstrt->Dst->V,
                             ConstraintKind::Subset)) {
    CSG.insertConstraint(Cstrt->Src->V, Cstrt->Dst->V, ConstraintKind::Subset);
    Worklist.push_back(Cstrt->Dst);
    return;
  }
  if (isa<Function>(Cstrt->Dst->V))
    return;
  for (auto *V : PointToSetMap[Cstrt->Src]) {
    auto *CSNodePointTo = CSG.getOrInsertConstraintNode(V);
    if (!CSG.hasConstraintEdge(Cstrt->Dst->V, CSNodePointTo->V,
                               ConstraintKind::Subset)) {
      CSG.insertConstraint(Cstrt->Dst->V, CSNodePointTo->V,
                           ConstraintKind::Subset);
      Worklist.push_back(CSNodePointTo);
    }
  }
}

void PointerAnalysisVisitor::solveStoreConstraint(const Constraint *Cstrt,
                                                  ConstraintGraphNode *CSNode) {
  if (PointToSetMap[Cstrt->Dst].empty() &&
      !CSG.hasConstraintEdge(Cstrt->Dst->V, Cstrt->Src->V,
                             ConstraintKind::Subset)) {
    CSG.insertConstraint(Cstrt->Dst->V, Cstrt->Src->V, ConstraintKind::Subset);
    Worklist.push_back(Cstrt->Src);
    return;
  }
  for (auto *V : PointToSetMap[Cstrt->Dst]) {
    auto *CSNodePointTo = CSG.getOrInsertConstraintNode(V);
    if (isa<Function>(CSNodePointTo->V))
      continue;
    if (!CSG.hasConstraintEdge(CSNodePointTo->V, Cstrt->Src->V,
                               ConstraintKind::Subset)) {
      CSG.insertConstraint(CSNodePointTo->V, Cstrt->Src->V,
                           ConstraintKind::Subset);
      Worklist.push_back(Cstrt->Src);
    }
  }
}

void PointerAnalysisVisitor::solveGetAddrConstraint(
    const Constraint *Cstrt, ConstraintGraphNode *CSNode) {
  auto PrevPointToSet = PointToSetMap[Cstrt->Src];
  PointToSetMap[Cstrt->Src].insert(CSNode->V);
  CSG.getOrInsertConstraint(Cstrt->Dst->V, CSNode->V, ConstraintKind::Subset);
  if (PrevPointToSet != PointToSetMap[Cstrt->Src]) {
    Worklist.push_back(Cstrt->Src);
  }
}

void PointerAnalysisVisitor::solveSubsetConstraint(
    const Constraint *Cstrt, ConstraintGraphNode *CSNode) {
  auto PrevPointToSet = PointToSetMap[Cstrt->Dst];
  PointToSetMap[Cstrt->Dst].insert(PointToSetMap[Cstrt->Src].begin(),
                                   PointToSetMap[Cstrt->Src].end());
  if (PrevPointToSet != PointToSetMap[Cstrt->Dst]) {
    Worklist.push_back(Cstrt->Dst);
  }
}

void PointerAnalysisVisitor::solveUnsolvedConstraint(
    const Constraint *Cstrt, ConstraintGraphNode *CSNode) {
  auto *Call = dyn_cast<CallInst>(Cstrt->Dst->V);
  assert(Call && "Dst should be a CallInst in a Unresolved constraint");
  for (auto *PointToValue : PointToSetMap[Cstrt->Src]) {
    if (auto *Callee = dyn_cast<Function>(PointToValue)) {
      DataflowResult<PAAnalysisDataflowFacts>::Type SubroutineResult;
      PAAnalysisDataflowFacts SubroutineInitFact;
      compForwardDataflow(Callee, this, &SubroutineResult, SubroutineInitFact);
      for (auto *V : FunctionReturnValueMap[Callee]) {
        CSG.getOrInsertConstraint(Cstrt->Dst->V, V, ConstraintKind::Subset);
        Worklist.push_back(CSG.getOrInsertConstraintNode(V));
      }
      if (Callee->getFunctionType()->getNumParams() ==
          UnresolvedArgs[Cstrt->Src].size()) {
        for (unsigned I = 0, NumOperands = static_cast<unsigned>(
                                 UnresolvedArgs[Cstrt->Src].size());
             I != NumOperands; ++I) {
          auto *RArgNode = UnresolvedArgs[Cstrt->Src][I];
          auto *FArgNode = CSG.getOrInsertConstraintNode(Callee->getArg(I));
          CSG.getOrInsertConstraint(FArgNode->V, RArgNode->V,
                                    ConstraintKind::Subset);
          if (RArgNode->V->getType()->isPointerTy()) {
            CSG.getOrInsertConstraint(RArgNode->V, FArgNode->V,
                                      ConstraintKind::Subset);
          }
          Worklist.push_back(RArgNode);
        }
      }
      auto *CGNode = CG[ConstraintFunctionMap[Cstrt]];
      bool Extend = false;
      for (auto CI = CGNode->begin(), CE = CGNode->end(); CI != CE; CI++) {
        auto *CallRecord = CI->second;
        if (CallRecord->getFunction() == Callee)
          Extend = true;
      }
      if (!Extend) {
        CGNode->addCalledFunction(Call, CG.getOrInsertFunction(Callee));
      }
    }
  }
}

/**
 * Removes redundant call edges from the call graph.
 */
void PointerAnalysisVisitor::removeRedundantCallEdge() {
  for (auto &Node : CG) {
    auto *CGNode = Node.second.get();
    SmallMapVector<Function *, unsigned, 4> CallCountsMap;
    for (auto CI = CGNode->begin(), CE = CGNode->end(); CI != CE; CI++) {
      auto *CallRecord = CI->second;
      CallCountsMap[CallRecord->getFunction()] = 0;
    }
    for (auto CI = CGNode->begin(), CE = CGNode->end(); CI != CE; CI++) {
      auto *CallRecord = CI->second;
      CallCountsMap[CallRecord->getFunction()] += 1;
      if (CallCountsMap[CallRecord->getFunction()] > 1)
        CGNode->removeCallEdge(CI);
    }
  }
}

std::array<std::string, static_cast<unsigned>(ConstraintKind::Init)>
    ConstraintKindToString = {"Subset", "GetAddr", "Load", "Store",
                              "Unresolve"};

/** * @brief Prints the constraint graph.
 *
 * This function prints the constraint graph in the DOT format. The graph
 * represents the constraints between different values in the analysis. The
 * constraints are grouped by function name and printed accordingly. If a value
 * has function information, it is grouped under the respective function name.
 *
 * @param OS The output stream to which the graph will be printed.
 */
void PointerAnalysisVisitor::printConstraintGraph(raw_ostream &OS) {
  OS << "digraph \"Constraint Graph\" {\n";

  // Map to group constraints by function name
  std::map<std::string, std::vector<std::string>> FunctionConstraints;

  for (const auto &Constraint : CSG.getConstraints()) {
    std::string SrcName = getShortValueName(Constraint->Src->V);
    std::string DstName = getShortValueName(Constraint->Dst->V);

    // Extract function names from source and destination
    size_t SrcFunctionIdx = SrcName.find(":");
    size_t DstFunctionIdx = DstName.find(":");

    std::string FunctionName;

    if (SrcFunctionIdx != std::string::npos) {
      FunctionName = SrcName.substr(SrcFunctionIdx + 1);
    } else if (DstFunctionIdx != std::string::npos) {
      FunctionName = DstName.substr(DstFunctionIdx + 1);
    }

    std::string ConstraintStr =
        "  \"" + SrcName + "\" -> \"" + DstName + "\" [label=\"" +
        ConstraintKindToString[static_cast<unsigned>(Constraint->Kind)] +
        "\"];\n";

    if (SrcFunctionIdx != std::string::npos &&
        DstFunctionIdx != std::string::npos &&
        SrcName.substr(SrcFunctionIdx + 1) ==
            DstName.substr(DstFunctionIdx + 1)) {
      // Both have function information and it's the same
      FunctionConstraints[FunctionName].push_back(ConstraintStr);
    } else if (!FunctionName.empty()) {
      // At least one has function information, so group by that
      FunctionConstraints[FunctionName].push_back(ConstraintStr);
    }
  }

  // Print grouped constraints by function
  for (const auto &Entry : FunctionConstraints) {
    OS << "// Function: " << Entry.first << "\n";
    for (const auto &Cstrt : Entry.second) {
      OS << Cstrt;
    }
  }

  OS << "}\n";
}

void PointerAnalysisVisitor::printPointToSetMap(raw_ostream &OS) {
  // Map to group PointToSet entries by function name
  std::map<std::string,
           std::vector<std::pair<std::string, std::vector<Value *>>>>
      FunctionPointSetMap;

  for (const auto &Map : PointToSetMap) {
    std::string VarName = getShortValueName(Map.first->V);

    // Extract function name from the variable name
    size_t FunctionIdx = VarName.find(":");
    if (FunctionIdx != std::string::npos) {
      std::string FunctionName = VarName.substr(FunctionIdx + 1);

      // Store the variable and its PointToSet values in the corresponding
      // function's group
      FunctionPointSetMap[FunctionName].emplace_back(
          VarName, std::vector<Value *>(Map.second.begin(), Map.second.end()));
    }
  }

  // Print grouped PointToSet entries by function
  for (const auto &Entry : FunctionPointSetMap) {
    OS << "// Function: " << Entry.first << "\n";
    for (const auto &VarAndPointToSet : Entry.second) {
      OS << VarAndPointToSet.first << ":";
      for (const auto &PointToValue : VarAndPointToSet.second) {
        OS << "  " << getShortValueName(PointToValue) << "";
      }
      OS << "\n";
    }
  }
}