//===- builtins/arrch64/matrix/sub_int2.c - sme matrix operations -*- C -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "sme_acle.h"

// Note: SME does not have native sub instruction for matrix type, we use SVE
// version.
__attribute__((target("+sve"))) void
__sme_sub_int2(signed short *dst, signed short *lhs, signed short *rhs,
               unsigned row, unsigned column) {
  SME_ACLE_BINOP_SVE(lhs, rhs, dst, row, column, h, int16, 16, s, sub);
  return;
}
