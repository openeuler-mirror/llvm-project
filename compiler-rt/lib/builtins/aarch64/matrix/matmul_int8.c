//=- builtins/arrch64/matrix/matmul_int8.c - sme matrix operations -*- C -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "sme_acle.h"

// Note: SME does not have native FMOPA instruction for i64 type, we use SVE
// version.
__attribute__((target("+sve"))) void
__sme_matmul_int8(int64_t *dst, int64_t *lhs, int64_t *rhs, unsigned lhs_row,
                  unsigned lhs_column, unsigned rhs_column) {
  SME_ACLE_MATMUL_SVE(lhs, rhs, dst, lhs_row, lhs_column, rhs_column, d, int64,
                      64, s);
  return;
}
