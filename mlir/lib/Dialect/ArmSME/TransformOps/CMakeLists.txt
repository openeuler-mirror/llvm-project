add_mlir_dialect_library(MLIRArmSMEVectorTransformOps
  ArmSMEVectorTransformOps.cpp

  ADDITIONAL_HEADER_DIRS
  ${MLIR_MAIN_INCLUDE_DIR}/mlir/Dialect/ArmSME/TransformOps

  DEPENDS
  MLIRArmSMEVectorTransformOpsIncGen

  LINK_LIBS PUBLIC
  MLIRArmSMEDialect
  MLIRArmSMETransforms
  MLIRIR
  MLIRLLVMCommonConversion
  MLIRLLVMDialect
  MLIRTransformDialect
  MLIRVectorDialect
  )
