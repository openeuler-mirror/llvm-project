//===- PromoteI1ToI8.h - Promote vector<N x i1> to vector<N x i8> -*- C++ -*=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef MLIR_DIALECT_LLVMIR_TRANSFORMS_PROMOTEI1TOI8_H
#define MLIR_DIALECT_LLVMIR_TRANSFORMS_PROMOTEI1TOI8_H

#include <memory>

namespace mlir {
class Pass;

namespace LLVM {

#define GEN_PASS_DECL_LLVMPROMOTEI1TOI8
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h.inc"

/// Creates a pass that promotes vector<N x i1> memory operations (loads and
/// stores) in the LLVM dialect to use vector<N x i8> instead.
///
/// When LLVM stores a vector<N x i1>, it packs the bits so that N elements
/// occupy only N/8 bytes. However, when individual i1 elements are later loaded
/// via scalar GEP + load i1, each element is addressed at byte granularity (1
/// byte per element). This mismatch causes wrong results because the scalar
/// loads read garbage for most elements.
///
/// The fix is to promote vector<N x i1> stores to vector<N x i8> (with zext
/// before store) and vector<N x i1> loads to vector<N x i8> (with trunc after
/// load), so that each boolean element occupies a full byte in memory.
std::unique_ptr<Pass> createPromoteI1ToI8Pass();

} // namespace LLVM
} // namespace mlir

#endif // MLIR_DIALECT_LLVMIR_TRANSFORMS_PROMOTEI1TOI8_H
