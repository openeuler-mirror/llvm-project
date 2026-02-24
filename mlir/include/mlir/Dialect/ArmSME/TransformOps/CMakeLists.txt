set(LLVM_TARGET_DEFINITIONS ArmSMEVectorTransformOps.td)
mlir_tablegen(ArmSMEVectorTransformOps.h.inc -gen-op-decls)
mlir_tablegen(ArmSMEVectorTransformOps.cpp.inc -gen-op-defs)
add_public_tablegen_target(MLIRArmSMEVectorTransformOpsIncGen)

add_mlir_doc(ArmSMEVectorTransformOps ArmSMEVectorTransformOps Dialects/ -gen-op-doc)
