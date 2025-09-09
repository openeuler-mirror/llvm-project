//===- builtins/arrch64/matrix/sub_int1.c - sme matrix operations -*- C -*-===//
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
__sme_sub_int1(signed char *dst, signed char *lhs, signed char *rhs,
               unsigned row, unsigned column) {
  SME_ACLE_BINOP_SVE(lhs, rhs, dst, row, column, b, int8, 8, s, sub);
  return;
}
