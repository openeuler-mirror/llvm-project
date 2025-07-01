//=- builtins/arrch64/matrix/matmul_uint1.c - sme matrix operations -*- C -*-=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "sme_acle.h"

// Note: SME does not have native FMOPA instruction for zext i8 type, we use SVE
// version.
__attribute__((target("+sve"))) void
__sme_matmul_uint1(unsigned char *dst, unsigned char *lhs, unsigned char *rhs,
                   unsigned lhs_row, unsigned lhs_column, unsigned rhs_column) {
  SME_ACLE_MATMUL_SVE(lhs, rhs, dst, lhs_row, lhs_column, rhs_column, b, uint8,
                      8, u);
  return;
}
