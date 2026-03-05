add_mlir_dialect_library(MLIRLLVMIRTransforms
  AddComdats.cpp
  DIExpressionLegalization.cpp
  DIExpressionRewriter.cpp
  DIScopeForLLVMFuncOp.cpp
  LegalizeFloat8Types.cpp
  LegalizeForExport.cpp
  OptimizeForNVVM.cpp
  PromoteI1ToI8.cpp
  RequestCWrappers.cpp

  DEPENDS
  MLIRLLVMPassIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRFuncDialect
  MLIRLLVMDialect
  MLIRPass
  MLIRTransforms
  MLIRNVVMDialect
  )
