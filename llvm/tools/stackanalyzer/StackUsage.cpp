//===--- StackUsage.cpp - Analyze the callgraph of a LLVM bitcode file using
// pointer analysis ----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "StackUsage.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/RandomNumberGenerator.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/VirtualFileSystem.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include <optional>

using namespace llvm;
using namespace llvm::sys;

namespace {

SmallString<128> generateUniqueName(StringRef Prefix) {
  auto TimePoint = std::chrono::system_clock::now();
  auto Duration = TimePoint.time_since_epoch();
  auto Millis =
      std::chrono::duration_cast<std::chrono::milliseconds>(Duration).count();

  static std::random_device RD;
  static std::mt19937 RNG(RD());
  std::uniform_int_distribution<unsigned> Dist(0, 999999);
  unsigned RandomNum = Dist(RNG);

  SmallString<128> UniqueName;
  raw_svector_ostream OS(UniqueName);
  OS << Prefix << "_" << format("%lld", Millis) << "_"
     << format("%06u", RandomNum);

  return UniqueName;
}

} // anonymous namespace

namespace llvm {

void parseStackSizeFromSU(Module &Module,
                          MapVector<const Function *, unsigned> &StackSizeMap,
                          StringRef AnalysisTarget) {
  std::string UniqueSuFilename = (generateUniqueName("su_file") + ".su").str();
  emitSUFile(UniqueSuFilename, Module, AnalysisTarget);

  auto BufferOrError = MemoryBuffer::getFile(UniqueSuFilename);
  if (std::error_code EC = BufferOrError.getError()) {
    errs() << "Error opening file " << UniqueSuFilename << ": " << EC.message()
           << "\n";
    return;
  }

  std::unique_ptr<MemoryBuffer> Buffer = std::move(BufferOrError.get());
  StringRef Content = Buffer->getBuffer();

  // Split the file content into lines
  SmallVector<StringRef, 16> Lines;
  Content.split(Lines, '\n');

  // Iterate through each line
  for (StringRef Line : Lines) {
    if (Line.trim().empty())
      continue; // Skip empty lines

    // Split the line by tabs
    SmallVector<StringRef, 4> Parts;
    Line.split(Parts, '\t', -1, false);

    if (Parts.size() < 3) {
      errs() << "Invalid format in line: " << Line << "\n";
      continue;
    }

    // Extract the function name and stack size
    StringRef FullFunctionName = Parts[0];
    StringRef StackSizeStr = Parts[1];

    // Parse the stack size
    unsigned StackSize;
    if (StackSizeStr.getAsInteger(10, StackSize)) {
      errs() << "Invalid stack size in line: " << Line << "\n";
      continue;
    }

    // Extract the function name (remove path and extension)
    StringRef FunctionName = sys::path::filename(FullFunctionName);
    FunctionName = FunctionName.rsplit(':').second;

    // Find the corresponding function in the module
    Function *F = Module.getFunction(FunctionName);
    if (!F) {
      errs() << "Function " << FunctionName << " not found in module\n";
      continue;
    }

    // Insert the function and its stack size into the map
    StackSizeMap[F] = StackSize;
  }

  // Remove the .su file
  auto EC = fs::remove(UniqueSuFilename);
  if (EC) {
    errs() << "Error removing SU file: " << EC.message() << "\n";
  }
}

void emitSUFile(StringRef SUFilename, Module &Module,
                StringRef TargetTripleInput) {
  std::string TargetTriple;
  if (TargetTripleInput.empty()) {
    TargetTriple = sys::getDefaultTargetTriple();
  } else {
    TargetTriple = Triple::normalize(TargetTripleInput);
  }

  if (TargetTripleInput.empty()) {
    std::string DefaultTriple = sys::getDefaultTargetTriple();
    TargetTriple = StringRef(DefaultTriple);
  } else {
    TargetTriple = TargetTripleInput;
  }

  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();

  std::string Error;
  auto *Target = TargetRegistry::lookupTarget(TargetTriple, Error);
  if (!Target) {
    errs() << "Error: " << Error << "\n";
    return;
  }

  auto *CPU = "generic";
  auto *Features = "";

  TargetOptions Opt;
  Opt.StackUsageOutput = SUFilename;
  auto RM = std::optional<Reloc::Model>();
  std::unique_ptr<TargetMachine> TargetMachine(
      Target->createTargetMachine(TargetTriple, CPU, Features, Opt, RM));

  Module.setDataLayout(TargetMachine->createDataLayout());
  Module.setTargetTriple(TargetTriple);

  std::string UniqueObjectFilename =
      (generateUniqueName("output-objectfile") + ".o").str();
  auto TempObjectFile = sys::fs::TempFile::create(UniqueObjectFilename);
  if (!TempObjectFile) {
    errs() << "Error creating temp object file: "
           << toString(TempObjectFile.takeError()) << "\n";
    return;
  }

  std::error_code EC;
  raw_fd_ostream Dest(TempObjectFile->FD, false);
  if (EC) {
    errs() << "Error opening file: " << EC.message() << "\n";
    return;
  }

  legacy::PassManager Pass;
  auto FileType = CGFT_ObjectFile;
  if (TargetMachine->addPassesToEmitFile(Pass, Dest, nullptr, FileType)) {
    errs() << "TargetMachine can't emit a file of this type\n";
    return;
  }

  Pass.run(Module);
  Dest.flush();

  // Discard the temporary object file
  if (auto Err = TempObjectFile->discard()) {
    errs() << "Error discarding object file: " << toString(std::move(Err))
           << "\n";
    return;
  }
}

void StackOverflowDetector::analyze(
    const CallGraph &CG,
    const MapVector<const Function *, unsigned> &StackSizes) {
  auto CGI = CG.begin();
  auto CGE = CG.end();
  for (; CGI != CGE; ++CGI) {
    Function *F = CGI->second->getFunction();
    if (!F || F->isDeclaration())
      continue;
    if (F->getName() == EntryFunction)
      break;
  }
  traverse(CGI->second->getFunction(), CG, StackSizes);
}

void StackOverflowDetector::printResults(raw_ostream &OS) const {
  if (OverflowPaths.empty()) {
    OS << "No potential stack overflow path found(limit:" << Threshold
       << " bytes).\n";
  } else {
    for (const auto &Path : OverflowPaths) {
      OS << "Potential stack overflow path found(limit:" << Threshold
         << " bytes): \n";
      OS << "CallStack:\n";
      for (auto *F : Path.CallStack) {
        OS << "  " << F->getName() << "\n";
      }
      OS << "Analysis:\n";
      if (Path.StackSize <= Threshold) {
        OS << "- Recursive call without proper base case check.\n";
        OS << "- Unbounded recursion may lead to stack overflow.\n";
      } else {
        OS << "- Stack usage exceeds the limit along the call stack.\n";
      }
    }
  }
}

bool StackOverflowDetector::evaluateCurrentPath() {
  unsigned CumulativeStackSize = 0;
  for (auto &Entry : PathStack) {
    CumulativeStackSize += Entry.second;
  }
  if (CumulativeStackSize > Threshold) {
    std::vector<const Function *> CallStack;
    for (auto &Entry : PathStack) {
      CallStack.push_back(Entry.first);
    }
    OverflowPaths.push_back(Path({CallStack, CumulativeStackSize}));
    return true;
  }
  return false;
}

bool StackOverflowDetector::traverse(
    Function *F, const CallGraph &CG,
    const MapVector<const Function *, unsigned> &StackSizes) {
  // Check for loop detection: if we revisit a node that is in the PathStack,
  // it's a loop
  if (PathStack.count(F)) {
    unsigned LoopStackSize = 0;
    for (auto PI = PathStack.find(F), PE = PathStack.end(); PI != PE; ++PI) {
      LoopStackSize += PI->second;
    }

    // If the loop's stack cost is zero, treat it as a single node and evaluate
    // current path
    if (LoopStackSize == 0) {
      return evaluateCurrentPath();
    }
    // Otherwise, consider it a potential overflow path
    std::vector<const Function *> CallStack;
    unsigned CumulativeStackSize = 0;
    for (auto &Entry : PathStack) {
      CallStack.push_back(Entry.first);
      CumulativeStackSize += Entry.second;
    }
    // Add the called function to the call stack to give better diagnostics
    CallStack.push_back(F);
    OverflowPaths.push_back(Path({CallStack, CumulativeStackSize}));
    return true;
  }

  Visited.insert(F);
  unsigned CurrentStackSize = StackSizes.lookup(F);
  PathStack.insert({F, CurrentStackSize});
  if (evaluateCurrentPath()) {
    return true;
  }
  auto *CGNode = CG[F];

  bool FindOverflowPath = false;
  for (auto &Callee : *CGNode) {
    Function *CalleeF = Callee.second->getFunction();
    if (CalleeF && !CalleeF->isDeclaration()) {
      FindOverflowPath = traverse(CalleeF, CG, StackSizes) || FindOverflowPath;
    }
  }

  PathStack.pop_back();
  return FindOverflowPath;
}
} // namespace llvm