set(LLVM_LINK_COMPONENTS
  Analysis
  BitReader
  Core
  Passes
  Support
  ${LLVM_TARGETS_TO_BUILD}
)

add_llvm_tool(stackanalyzer
  CallGraphGen.cpp
  stackanalyzer.cpp   
  StackUsage.cpp
)