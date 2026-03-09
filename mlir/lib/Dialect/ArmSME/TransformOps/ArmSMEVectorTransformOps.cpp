//===- ArmSMEVectorTransformOps.cpp - Implementation transform ops --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/ArmSME/TransformOps/ArmSMEVectorTransformOps.h"

#include "mlir/Dialect/ArmSME/IR/ArmSME.h"
#include "mlir/Dialect/ArmSME/Transforms/Transforms.h"
#include "mlir/Dialect/Transform/IR/TransformDialect.h"

using namespace mlir;

#define GET_OP_CLASSES
#include "mlir/Dialect/ArmSME/TransformOps/ArmSMEVectorTransformOps.cpp.inc"

//===----------------------------------------------------------------------===//
// Transform op registration
//===----------------------------------------------------------------------===//

namespace {
class ArmSMEVectorTransformDialectExtension
    : public transform::TransformDialectExtension<
          ArmSMEVectorTransformDialectExtension> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(
      ArmSMEVectorTransformDialectExtension)

  ArmSMEVectorTransformDialectExtension() {
    declareGeneratedDialect<arm_sme::ArmSMEDialect>();
    registerTransformOps<
#define GET_OP_LIST
#include "mlir/Dialect/ArmSME/TransformOps/ArmSMEVectorTransformOps.cpp.inc"
        >();
  }
};
} // namespace

void mlir::arm_sme::registerTransformDialectExtension(
    DialectRegistry &registry) {
  registry.addExtensions<ArmSMEVectorTransformDialectExtension>();
}
