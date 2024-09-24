#include "CallGraphGen.h"
#include "StackUsage.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include <memory>
#include <string>

using namespace llvm;

static cl::OptionCategory StackAnalyzerCategory("StackAnalyzerCategory");

static cl::opt<std::string>
    InputFilename(cl::Positional, cl::desc("Input .bc file to be analyzed"),
                  cl::cat(StackAnalyzerCategory));

static cl::opt<std::string>
    AnalysisTarget("target",
                   cl::desc("The target backend for call stack analysis"),
                   cl::init(""), cl::cat(StackAnalyzerCategory));

static cl::opt<bool>
    UseCallGraph("callgraph",
                 cl::desc("Output the callgraph given the .bc file"),
                 cl::cat(StackAnalyzerCategory));

static cl::opt<bool>
    UseAnalysis("analysis",
                cl::desc("Output possible path of the callgraph which can "
                         "possibly cause stack overflow"),
                cl::cat(StackAnalyzerCategory));

static cl::opt<unsigned>
    LimitSize("stacksize",
              cl::desc("Max stack size of the limit of a path within the "
                       "callgraph, given the .bc file. "
                       "Should be used together with `analysis`."),
              cl::init(1024), cl::cat(StackAnalyzerCategory));

static cl::opt<bool>
    UseAnders("anders",
              cl::desc("Use Anders analysis to analyze the call graph"),
              cl::init(false), cl::cat(StackAnalyzerCategory));

static cl::opt<std::string> OutputFilename(
    "o",
    cl::desc("Output callgraph in .dot format with stack cost information"
             "Should be used together with `analysis`."),
    cl::init(""), cl::cat(StackAnalyzerCategory));

static cl::opt<std::string>
    EntryFunction("entry",
                  cl::desc("The name of the entry function for the callgraph"),
                  cl::init("main"), cl::cat(StackAnalyzerCategory));

// Hidden options
static cl::opt<bool>
    UseDebug("debuginfo",
             cl::desc("Enable debug output for the call graph analysis"),
             cl::cat(StackAnalyzerCategory), cl::Hidden);

static Expected<std::unique_ptr<MemoryBuffer>> openBitcodeFile(StringRef Path) {
  Expected<std::unique_ptr<MemoryBuffer>> MemBufOrErr =
      errorOrToExpected(MemoryBuffer::getFileOrSTDIN(Path));
  if (Error E = MemBufOrErr.takeError())
    return E;

  std::unique_ptr<MemoryBuffer> MemBuf = std::move(*MemBufOrErr);

  return MemBuf;
}

int main(int argc, char **argv) {
  InitLLVM X(argc, argv);
  cl::HideUnrelatedOptions(StackAnalyzerCategory);
  cl::ParseCommandLineOptions(argc, argv);
  ExitOnError ExitOnErr("stackanalyzer: ");

  LLVMContext Context;
  auto MB = ExitOnErr(openBitcodeFile(InputFilename));
  auto M = ExitOnErr(parseBitcodeFile(MB->getMemBufferRef(), Context));

  auto Config = PointerAnalysisCLIConfig{UseAnders, UseDebug, EntryFunction};

  ModuleAnalysisManager MAM;
  PassBuilder PB;
  PB.registerModuleAnalyses(MAM);
  MAM.registerPass([Config] { return PACallGraphAnalysis(Config); });
  ModulePassManager MPM;
  MPM.addPass(RequireAnalysisPass<PACallGraphAnalysis, Module>());
  MPM.run(*M, MAM);

  MapVector<const Function *, unsigned> StackSize;
  for (auto &F : *M) {
    StackSize.insert(std::make_pair(&F, 0));
  }
  parseStackSizeFromSU(*M, StackSize, AnalysisTarget);

  const auto &Graph = MAM.getResult<PACallGraphAnalysis>(*M);

  if (UseCallGraph) {
    if (!OutputFilename.empty()) {
      std::error_code EC;
      raw_fd_ostream File(OutputFilename, EC, sys::fs::OF_Text);
      if (!EC) {
        File << "digraph \"CallGraph\" {\n";
        for (auto &NodePair : Graph) {
          CallGraphNode *Node = NodePair.second.get();
          if (Function *F = Node->getFunction()) {
            if (F->isIntrinsic())
              continue;
            File << "  \"" << F->getName() << "\";\n";
          }
        }
        for (auto &NodePair : Graph) {
          CallGraphNode *Node = NodePair.second.get();
          if (Function *F = Node->getFunction()) {
            if (F->isIntrinsic())
              continue;
            File << "  \"" << F->getName() << "\" [label=\"" << F->getName()
                 << "\\nStack Size: " << StackSize[F] << " bytes\"];\n";
          }
        }
        for (auto &NodePair : Graph) {
          CallGraphNode *Node = NodePair.second.get();
          if (Function *F = Node->getFunction()) {
            if (F->isIntrinsic())
              continue;
            for (auto &CallRecord : *Node) {
              if (Function *Callee = CallRecord.second->getFunction()) {
                if (Callee->isIntrinsic())
                  continue;
                File << "  \"" << F->getName() << "\" -> \""
                     << Callee->getName() << "\";\n";
              }
            }
          }
        }
        File << "}\n";
      }
    } else {
      Graph.print(outs());
    }
  }
  if (UseAnalysis) {
    StackOverflowDetector Detector{LimitSize, EntryFunction};
    Detector.analyze(Graph, StackSize);
    Detector.printResults(outs());
  }
  return 0;
}