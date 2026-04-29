//===-- llvm-split: command line tool for testing module splitter ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This program can be used to test the llvm::SplitModule function.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/StringExtras.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/LTO/Config.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/ThreadPool.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/Utils/SplitModule.h"
#include "llvm/Transforms/Utils/SplitModuleCG.h"

using namespace llvm;

static cl::OptionCategory SplitCategory("Split Options");

static cl::opt<std::string> InputFilename(cl::Positional,
                                          cl::desc("<input bitcode file>"),
                                          cl::init("-"),
                                          cl::value_desc("filename"),
                                          cl::cat(SplitCategory));

static cl::opt<std::string> OutputFilename("o",
                                           cl::desc("Override output filename"),
                                           cl::value_desc("filename"),
                                           cl::cat(SplitCategory));

static cl::opt<unsigned> NumOutputs("j", cl::Prefix, cl::init(2),
                                    cl::desc("Number of output files"),
                                    cl::cat(SplitCategory));

static cl::opt<bool>
    PreserveLocals("preserve-locals", cl::Prefix, cl::init(false),
                   cl::desc("Split without externalizing locals"),
                   cl::cat(SplitCategory));

static cl::opt<bool>
    EnableSplitModuleCG("enable-split-module-CG", cl::Prefix, cl::init(false),
                        cl::desc("Split module using call graph"),
                        cl::cat(SplitCategory));

int main(int argc, char **argv) {
  LLVMContext Context;
  SMDiagnostic Err;
  cl::HideUnrelatedOptions({&SplitCategory, &getColorCategory()});
  cl::ParseCommandLineOptions(argc, argv, "LLVM module splitter\n");

  std::unique_ptr<Module> M = parseIRFile(InputFilename, Err, Context);

  if (!M) {
    Err.print(argv[0], errs());
    return 1;
  }

  if (EnableSplitModuleCG) {
    const auto HandleModulePartCG = [&](std::unique_ptr<Module> MPart, unsigned I) {
      std::error_code EC;
      std::unique_ptr<ToolOutputFile> Out(
          new ToolOutputFile(OutputFilename + utostr(I), EC, sys::fs::OF_None));
      if (EC) {
        errs() << EC.message() << '\n';
        exit(1);
      }

      if (verifyModule(*MPart, &errs())) {
        errs() << "Broken module!\n";
        exit(1);
      }

      WriteBitcodeToFile(*MPart, Out->os());

      // Declare success.
      Out->keep();
    };

    llvm::lto::Config Config;
    ModuleSummaryIndex CombinedIndex(false);
    std::unique_ptr<TargetMachine> TM;
    SplitModuleCG SplitModuleCG(*M, Config, CombinedIndex, NumOutputs);
    SplitModuleCG.SplitModule(TM.get(), HandleModulePartCG, false);
    return 0;
  }

  unsigned I = 0;
  SplitModule(
      *M, NumOutputs,
      [&](std::unique_ptr<Module> MPart) {
        std::error_code EC;
        std::unique_ptr<ToolOutputFile> Out(new ToolOutputFile(
            OutputFilename + utostr(I++), EC, sys::fs::OF_None));
        if (EC) {
          errs() << EC.message() << '\n';
          exit(1);
        }

        if (verifyModule(*MPart, &errs())) {
          errs() << "Broken module!\n";
          exit(1);
        }

        WriteBitcodeToFile(*MPart, Out->os());

        // Declare success.
        Out->keep();
      },
      PreserveLocals);

  return 0;
}
