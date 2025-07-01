//==- builtins/arrch64/matrix/add_uint2.c - sme matrix operations -*- C -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "sme_acle.h"

// Note: SME does not have native add instruction for matrix type, we use SVE
// version.
__attribute__((target("+sve"))) void
__sme_add_uint2(unsigned short *dst, unsigned short *lhs, unsigned short *rhs,
                unsigned row, unsigned column) {
  SME_ACLE_BINOP_SVE(lhs, rhs, dst, row, column, h, uint16, 16, u, add);
  return;
}
