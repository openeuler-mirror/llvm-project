add_mlir_dialect_library(MLIRArmSMEVectorTransformOps
  ArmSMEVectorTransformOps.cpp

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/ArmSME/TransformOps

  DEPENDS
  MLIRArmSMEVectorTransformOpsIncGen

  LINK_LIBS PUBLIC
  MLIRIR
  MLIRLLVMCommonConversion
  MLIRLLVMDialect
  MLIRVectorDialect
  MLIRTransformDialect
  MLIRArmSMEDialect
  MLIRArmSMETransforms
  )
