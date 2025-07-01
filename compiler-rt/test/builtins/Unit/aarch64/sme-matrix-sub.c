// REQUIRES: aarch64-target-arch,aarch64_sme_run

// RUN: %clang_builtins -march=armv9.1-a+sme -fenable-matrix %s %librt -o %t
// RUN: %run %t 2>&1 | FileCheck %s
// RUN: %clang_builtins -march=armv9.1-a+sme -fenable-matrix %s %librt -O3 -o %t_smeopt
// RUN: %run %t_smeopt 2>&1 | FileCheck %s
// RUN: %clang_builtins -O3 -march=armv9.1-a -fenable-matrix %s %librt -o %t_nosmeopt
// RUN: %run %t_nosmeopt 2>&1 | FileCheck %s

#include <stdio.h>

#define M 2
#define N 4

typedef __bf16 m_tbf16 __attribute__((matrix_type(M, N)));

typedef float m_tfloat __attribute__((matrix_type(M, N)));

typedef double m_tdouble __attribute__((matrix_type(M, N)));

typedef signed int m_tint __attribute__((matrix_type(M, N)));

typedef unsigned long long m_tull __attribute__((matrix_type(M, N)));

int main() {
  m_tbf16 a;
  m_tbf16 b;
  float v = 0.0;

  // Input matrix 1:
  // 0.00 1.00 2.00 3.00
  // 4.00 5.00 6.00 7.00
  //
  // Input matrix 2:
  // 8.00  9.00  10.00 11.00
  // 12.00 13.00 14.00 15.00
  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      a[i][j] = v++;

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      b[i][j] = v++;

  m_tbf16 c = a - b;

  // CHECK:      -8.00 -8.00 -8.00 -8.00
  // CHECK-NEXT: -8.00 -8.00 -8.00 -8.00
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%.2f ", (float)c[i][j]);
    printf("\n");
  }

  m_tfloat af;
  m_tfloat bf;
  float vf = 0.0;

  // Input matrix 1:
  // 0.00 1.00 2.00 3.00
  // 4.00 5.00 6.00 7.00
  //
  // Input matrix 2:
  // 8.00  9.00  10.00 11.00
  // 12.00 13.00 14.00 15.00
  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      af[i][j] = vf++;

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      bf[i][j] = vf++;

  m_tfloat cf = af - bf;

  // CHECK:      -8.00 -8.00 -8.00 -8.00
  // CHECK-NEXT: -8.00 -8.00 -8.00 -8.00
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%.2f ", cf[i][j]);
    printf("\n");
  }

  m_tdouble ad;
  m_tdouble bd;
  double vd = 0.0;

  // Input matrix 1:
  // 0.00 1.00 2.00 3.00
  // 4.00 5.00 6.00 7.00
  //
  // Input matrix 2:
  // 8.00  9.00  10.00 11.00
  // 12.00 13.00 14.00 15.00
  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      ad[i][j] = vd++;

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      bd[i][j] = vd++;

  m_tdouble cd = ad - bd;

  // CHECK:      -8.00 -8.00 -8.00 -8.00
  // CHECK-NEXT: -8.00 -8.00 -8.00 -8.00
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%.2f ", cd[i][j]);
    printf("\n");
  }

  m_tint ai;
  m_tint bi;
  int vi = 0;

  // Input matrix 1:
  // 0 1 2 3
  // 4 5 6 7
  //
  // Input matrix 2:
  // 8  9  10 11
  // 12 13 14 15
  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      ai[i][j] = vi++;

  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      bi[i][j] = vi++;

  m_tint ci = ai - bi;

  // CHECK:      -8 -8 -8 -8
  // CHECK-NEXT: -8 -8 -8 -8
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%d ", ci[i][j]);
    printf("\n");
  }

  m_tull au;
  m_tull bu;
  unsigned long long int vu = 0;

  // Input matrix 1:
  // 0 1 2 3
  // 4 5 6 7
  //
  // Input matrix 2:
  // 0 1 2 3
  // 4 5 6 7
  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      au[i][j] = vu++;

  vu = 0;
  for (int i = 0; i < M; i++)
    for (int j = 0; j < N; j++)
      bu[i][j] = vu++;

  m_tull cu = au - bu;

  // CHECK:      0 0 0 0
  // CHECK-NEXT: 0 0 0 0
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%lld ", cu[i][j]);
    printf("\n");
  }

  return 0;
}
