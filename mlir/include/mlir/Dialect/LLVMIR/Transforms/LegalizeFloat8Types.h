//===- LegalizeFloat8Types.h - Replace f8 with i8 in LLVM dialect -*- C++ -*===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_DIALECT_LLVMIR_TRANSFORMS_LEGALIZEFLOAT8TYPES_H
#define MLIR_DIALECT_LLVMIR_TRANSFORMS_LEGALIZEFLOAT8TYPES_H

#include <memory>

namespace mlir {
class Pass;

namespace LLVM {

#define GEN_PASS_DECL_LLVMLEGALIZEFLOAT8TYPES
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h.inc"

/// Creates a pass that replaces all Float8 types in LLVM dialect operations
/// with i8 (integer 8-bit) types, since LLVM IR has no native Float8 types.
/// This pass should be run just before mlir-translate --mlir-to-llvmir.
std::unique_ptr<Pass> createLegalizeFloat8TypesPass();

} // namespace LLVM
} // namespace mlir

#endif // MLIR_DIALECT_LLVMIR_TRANSFORMS_LEGALIZEFLOAT8TYPES_H
