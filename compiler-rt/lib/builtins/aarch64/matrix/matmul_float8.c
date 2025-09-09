//= builtins/arrch64/matrix/matmul_float8.c - sme matrix operations -*- C -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "sme_acle.h"

__attribute__((target("+sme,+sve,+sme-f64f64"))) __arm_new(
    "za") void __sme_matmul_float8(double *dst, double *lhs, double *rhs,
                                   unsigned lhs_row, unsigned lhs_column,
                                   unsigned rhs_column) __arm_streaming {
  BREAK_SME_ACLE_MATMUL_SME(lhs, rhs, dst, lhs_row, lhs_column, rhs_column, d,
                            float64, 64, 0b00000001);
  return;
}
