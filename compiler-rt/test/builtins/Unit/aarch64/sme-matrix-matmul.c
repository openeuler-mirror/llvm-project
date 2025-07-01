// REQUIRES: aarch64-target-arch,aarch64_sme_run

// RUN: %clang_builtins -march=armv9.1-a+sme -fenable-matrix %s %librt -o %t
// RUN: %run %t 2>&1 | FileCheck %s
// RUN: %clang_builtins -march=armv9.1-a+sme -fenable-matrix %s %librt -O3 -o %t_smeopt
// RUN: %run %t_smeopt 2>&1 | FileCheck %s
// RUN: %clang_builtins -O3 -march=armv9.1-a -fenable-matrix %s %librt -o %t_nosmeopt
// RUN: %run %t_nosmeopt 2>&1 | FileCheck %s

#include <stdio.h>

// First matrix is 2x3. Second matrix is 3x4. Result matrix is 2x4.
#define M 2
#define K 3
#define N 4

typedef __bf16 m1_tbf16 __attribute__((matrix_type(M, K)));
typedef __bf16 m2_tbf16 __attribute__((matrix_type(K, N)));
typedef __bf16 mr_tbf16 __attribute__((matrix_type(M, N)));

typedef float m1_tfloat __attribute__((matrix_type(M, K)));
typedef float m2_tfloat __attribute__((matrix_type(K, N)));
typedef float mr_tfloat __attribute__((matrix_type(M, N)));

typedef double m1_tdouble __attribute__((matrix_type(M, K)));
typedef double m2_tdouble __attribute__((matrix_type(K, N)));
typedef double mr_tdouble __attribute__((matrix_type(M, N)));

typedef signed int m1_tint __attribute__((matrix_type(M, K)));
typedef signed int m2_tint __attribute__((matrix_type(K, N)));
typedef signed int mr_tint __attribute__((matrix_type(M, N)));

typedef unsigned long long m1_tull __attribute__((matrix_type(M, K)));
typedef unsigned long long m2_tull __attribute__((matrix_type(K, N)));
typedef unsigned long long mr_tull __attribute__((matrix_type(M, N)));

int main() {
  m1_tbf16 a;
  m2_tbf16 b;
  float v = 0.0;

  // Input matrix 1:
  // 0.00 1.00 2.00
  // 3.00 4.00 5.00
  //
  // Input matrix 2:
  // 6.00  7.00  8.00  9.00
  // 10.00 11.00 12.00 13.00
  // 14.00 15.00 16.00 17.00
  for (int i = 0; i < M; i++)
    for (int j = 0; j < K; j++)
      a[i][j] = v++;

  for (int i = 0; i < K; i++)
    for (int j = 0; j < N; j++)
      b[i][j] = v++;

  mr_tbf16 c = a * b;

  // CHECK:      38.00  41.00  44.00  47.00
  // CHECK-NEXT: 128.00 140.00 152.00 164.00
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%.2f ", (float)c[i][j]);
    printf("\n");
  }

  m1_tfloat af;
  m2_tfloat bf;
  float vf = 0.0;

  // Input matrix 1:
  // 0.00 1.00 2.00
  // 3.00 4.00 5.00
  //
  // Input matrix 2:
  // 6.00  7.00  8.00  9.00
  // 10.00 11.00 12.00 13.00
  // 14.00 15.00 16.00 17.00
  for (int i = 0; i < M; i++)
    for (int j = 0; j < K; j++)
      af[i][j] = vf++;

  for (int i = 0; i < K; i++)
    for (int j = 0; j < N; j++)
      bf[i][j] = vf++;

  mr_tfloat cf = af * bf;

  // CHECK:      38.00  41.00  44.00  47.00
  // CHECK-NEXT: 128.00 140.00 152.00 164.00
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%.2f ", cf[i][j]);
    printf("\n");
  }

  m1_tdouble ad;
  m2_tdouble bd;
  double vd = 0.0;

  // Input matrix 1:
  // 0.00 1.00 2.00
  // 3.00 4.00 5.00
  //
  // Input matrix 2:
  // 6.00  7.00  8.00  9.00
  // 10.00 11.00 12.00 13.00
  // 14.00 15.00 16.00 17.00
  for (int i = 0; i < M; i++)
    for (int j = 0; j < K; j++)
      ad[i][j] = vd++;

  for (int i = 0; i < K; i++)
    for (int j = 0; j < N; j++)
      bd[i][j] = vd++;

  mr_tdouble cd = ad * bd;

  // CHECK:      38.00  41.00  44.00  47.00
  // CHECK-NEXT: 128.00 140.00 152.00 164.00
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%.2f ", cd[i][j]);
    printf("\n");
  }

  m1_tint ai;
  m2_tint bi;
  int vi = 0;

  // Input matrix 1:
  // 0 1 2
  // 3 4 5
  //
  // Input matrix 2:
  // 6  7  8  9
  // 10 11 12 13
  // 14 15 16 17
  for (int i = 0; i < M; i++)
    for (int j = 0; j < K; j++)
      ai[i][j] = vi++;

  for (int i = 0; i < K; i++)
    for (int j = 0; j < N; j++)
      bi[i][j] = vi++;

  mr_tint ci = ai * bi;

  // CHECK:      38  41  44  47
  // CHECK-NEXT: 128 140 152 164
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%d ", ci[i][j]);
    printf("\n");
  }

  m1_tull au;
  m2_tull bu;
  unsigned long long int vu = 0;

  // Input matrix 1:
  // 0 1 2
  // 3 4 5
  //
  // Input matrix 2:
  // 6  7  8  9
  // 10 11 12 13
  // 14 15 16 17
  for (int i = 0; i < M; i++)
    for (int j = 0; j < K; j++)
      au[i][j] = vu++;

  for (int i = 0; i < K; i++)
    for (int j = 0; j < N; j++)
      bu[i][j] = vu++;

  mr_tull cu = au * bu;

  // CHECK:      38  41  44  47
  // CHECK-NEXT: 128 140 152 164
  for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++)
      printf("%lld ", cu[i][j]);
    printf("\n");
  }

  return 0;
}
