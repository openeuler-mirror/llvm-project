set(LLVM_LINK_COMPONENTS
  DebugInfoLogicalView
  )

add_llvm_unittest(DebugInfoLogicalViewTests
  CommandLineOptionsTest.cpp
  SelectElementsTest.cpp
  LocationRangesTest.cpp
  LogicalElementsTest.cpp
  StringPoolTest.cpp
  )

target_link_libraries(DebugInfoLogicalViewTests PRIVATE LLVMTestingSupport)
