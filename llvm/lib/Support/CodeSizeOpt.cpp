// CodeSizeOpt.cpp 

#include "llvm/Support/CodeSizeOpt.h"

namespace llvm {
cl::opt<bool> EnableCodeSize(
    "enable-code-size", cl::init(true), cl::Hidden,
    cl::desc("Enable optimizations for code size as part of the optimization "
             "pipeline"));
} // namespace llvm
