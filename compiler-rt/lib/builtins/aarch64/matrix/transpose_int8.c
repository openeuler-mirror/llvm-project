//===- builtins/arrch64/matrix/transpose_int8.c - sme matrix operations C -===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "sme_acle.h"

// For transpose, signed or unsigned does not matter.
__attribute__((target("+sme,+sve"))) __arm_new("za") void __sme_transpose_int8(
    long long *dst, long long *src, unsigned row,
    unsigned column) __arm_streaming {
  BREAK_SME_ACLE_TRANSPOSE(src, row, column, dst, d, 64, 0b00000001);
  return;
}
