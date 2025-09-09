//===- builtins/arrch64/matrix/sme_acle.h - sme matrix operations -*- C -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// This file implements the runtime SME routines for matrix_type operations.
//
//===----------------------------------------------------------------------===//

#ifndef _SME_ACLE_H_
#define _SME_ACLE_H_

#include <arm_sme.h>
#include <arm_sve.h>
#include <stddef.h>

#define SVCNT(x) svcnt##x
#define VectorType(x) sv##x##_t
#define SVWHILELT(x) svwhilelt_b##x##_u32
#define SMZERO svzero_mask_za
#define SMLD1H(x) svld1_hor_za##x
#define SMST1V(x) svst1_ver_za##x
#define SVLD1_FLOAT(x) svld1_f##x
#define SMMOPA_FLOAT(x) svmopa_za##x##_f##x##_m
#define SMEXTRACTV_FLOAT(x) svread_ver_za##x##_f##x##_m

#define SVDUP(type, bit) svdup_##type##bit
#define SVLD1(type, bit) svld1_##type##bit
#define SVMLA(type, bit) svmla_##type##bit##_x
#define SVST1(type, bit) svst1_##type##bit
#define SVBINOP(type, bit, op) sv##op##_##type##bit##_x

// We don't have add/sub etc. binary operations for za tiles. Use SVE version
// instead.
#define SME_ACLE_BINOP_SVE(matA, matB, matC, M, N, svcnt_type, vec_type, bit,  \
                           func_type, op_type)                                 \
  do {                                                                         \
    uint64_t vscale = SVCNT(svcnt_type)();                                     \
    VectorType(vec_type) src1, src2, res;                                      \
    svbool_t p;                                                                \
    for (size_t i = 0; i < M; i += vscale) {                                   \
      p = SVWHILELT(bit)(i, M);                                                \
      for (size_t j = 0; j < N; j++) {                                         \
        src1 = SVLD1(func_type, bit)(p, matA + j * M + i);                     \
        src2 = SVLD1(func_type, bit)(p, matB + j * M + i);                     \
        res = SVBINOP(func_type, bit, op_type)(p, src1, src2);                 \
        SVST1(func_type, bit)(p, matC + j * M + i, res);                       \
      }                                                                        \
    }                                                                          \
  } while (0)

// We don't have non-widening matmul for types except float4 and float8 in SME.
// Use SVE version instead.
#define SME_ACLE_MATMUL_SVE(matA, matB, matC, M, K, N, svcnt_type, vec_type,   \
                            bit, func_type)                                    \
  do {                                                                         \
    uint64_t vscale = SVCNT(svcnt_type)();                                     \
    VectorType(vec_type) src1, src2, acc;                                      \
    svbool_t p;                                                                \
    for (size_t j = 0; j < N; j++)                                             \
      for (size_t i = 0; i < M; i += vscale) {                                 \
        acc = SVDUP(func_type, bit)(0);                                        \
        p = SVWHILELT(bit)(i, M);                                              \
        for (size_t k = 0; k < K; ++k) {                                       \
          src1 = SVDUP(func_type, bit)(matB[j * K + k]);                       \
          src2 = SVLD1(func_type, bit)(p, matA + k * M + i);                   \
          acc = SVMLA(func_type, bit)(p, acc, src1, src2);                     \
        }                                                                      \
        SVST1(func_type, bit)(p, matC + j * M + i, acc);                       \
      }                                                                        \
  } while (0)

// matrix_type in clang is column major, so we can just reuse Fortran's matmul
// version.
#define BREAK_SME_ACLE_MATMUL_SME(matA, matB, matC, M, K, N, svcnt_type,       \
                                  vec_type, bit, zero_mask)                    \
  do {                                                                         \
    uint64_t vscale = SVCNT(svcnt_type)();                                     \
    svbool_t pm, pn, pk;                                                       \
    VectorType(vec_type) src1, src2;                                           \
    for (size_t i = 0; i < M; i += vscale) {                                   \
      pm = SVWHILELT(bit)(i, M);                                               \
      for (size_t j = 0; j < N; j += vscale) {                                 \
        pn = SVWHILELT(bit)(j, N);                                             \
        SMZERO(zero_mask);                                                     \
        for (size_t k = 0; k < K; k += vscale) {                               \
          pk = SVWHILELT(bit)(k, K);                                           \
          for (size_t t = 0; t < vscale; t++) {                                \
            if (j + t == N)                                                    \
              break;                                                           \
            SMLD1H(bit)(1, t, pk, matB + (j + t) * K + k);                     \
          }                                                                    \
          for (size_t t = 0; t < vscale; t++) {                                \
            if (k + t == K)                                                    \
              break;                                                           \
            src2 = SMEXTRACTV_FLOAT(bit)(src2, pn, 1, t);                      \
            src1 = SVLD1_FLOAT(bit)(pm, matA + (k + t) * M + i);               \
            SMMOPA_FLOAT(bit)(0, pm, pn, src1, src2);                          \
          }                                                                    \
        }                                                                      \
        for (size_t t = 0; t < vscale; t++) {                                  \
          if (j + t == N)                                                      \
            break;                                                             \
          SMST1V(bit)(0, t, pm, matC + (j + t) * M + i);                       \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)

// matrix_type in clang is column major, so we can just reuse Fortran's
// transpose version.
#define BREAK_SME_ACLE_TRANSPOSE(matA, M, N, ans, svcnt_type, bit, zero_mask)  \
  do {                                                                         \
    uint64_t vscale = SVCNT(svcnt_type)();                                     \
    svbool_t pm, pn;                                                           \
    for (size_t i = 0; i < M; i += vscale) {                                   \
      pm = SVWHILELT(bit)(i, M);                                               \
      for (size_t j = 0; j < N; j += vscale) {                                 \
        pn = SVWHILELT(bit)(j, N);                                             \
        SMZERO(zero_mask);                                                     \
        for (size_t t = 0; t < vscale; t++) {                                  \
          if (j + t == N)                                                      \
            break;                                                             \
          SMLD1H(bit)(0, t, pm, matA + (j + t) * M + i);                       \
        }                                                                      \
        for (size_t t = 0; t < vscale; t++) {                                  \
          if (i + t == M)                                                      \
            break;                                                             \
          SMST1V(bit)(0, t, pn, ans + (i + t) * N + j);                        \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  } while (0)

#endif
