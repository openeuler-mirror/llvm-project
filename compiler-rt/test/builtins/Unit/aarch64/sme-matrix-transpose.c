// REQUIRES: aarch64-target-arch,aarch64_sme_run

// RUN: %clang_builtins -march=armv9.1-a+sme -fenable-matrix %s %librt -o %t
// RUN: %run %t 2>&1 | FileCheck %s
// RUN: %clang_builtins -march=armv9.1-a+sme -fenable-matrix %s %librt -O3 -o %t_smeopt
// RUN: %run %t_smeopt 2>&1 | FileCheck %s
// RUN: %clang_builtins -O3 -march=armv9.1-a -fenable-matrix %s %librt -o %t_nosmeopt
// RUN: %run %t_nosmeopt 2>&1 | FileCheck %s

#include <stdio.h>

#define ROW 2
#define COL 3

typedef __bf16 m_t __attribute__((matrix_type(ROW, COL)));
typedef __bf16 mt_t __attribute__((matrix_type(COL, ROW)));

typedef double md_t __attribute__((matrix_type(ROW, COL)));
typedef double mdt_t __attribute__((matrix_type(COL, ROW)));

typedef signed char mc_t __attribute__((matrix_type(ROW, COL)));
typedef signed char mct_t __attribute__((matrix_type(COL, ROW)));

typedef unsigned int mui_t __attribute__((matrix_type(ROW, COL)));
typedef unsigned int muit_t __attribute__((matrix_type(COL, ROW)));

int main() {
  m_t a;
  float v = 0.0;

  // Input matrix:
  // 0.00 1.00 2.00
  // 3.00 4.00 5.00
  for (int i = 0; i < ROW; i++)
    for (int j = 0; j < COL; j++)
      a[i][j] = v++;

  mt_t b = __builtin_matrix_transpose(a);

  // CHECK:      0.00 3.00
  // CHECK-NEXT: 1.00 4.00
  // CHECK-NEXT: 2.00 5.00
  for (int i = 0; i < COL; i++) {
    for (int j = 0; j < ROW; j++)
      printf("%.2f ", (float)b[i][j]);
    printf("\n");
  }

  md_t ad;
  double vd = 1.0;

  // Input matrix:
  // 1.00 2.00 3.00
  // 4.00 5.00 6.00
  for (int i = 0; i < ROW; i++)
    for (int j = 0; j < COL; j++)
      ad[i][j] = vd++;

  mdt_t bd = __builtin_matrix_transpose(ad);

  // CHECK:      1.00 4.00
  // CHECK-NEXT: 2.00 5.00
  // CHECK-NEXT: 3.00 6.00
  for (int i = 0; i < COL; i++) {
    for (int j = 0; j < ROW; j++)
      printf("%.2f ", bd[i][j]);
    printf("\n");
  }

  mc_t ac;
  signed char vc = 5;

  // Input matrix:
  // 5 6 7
  // 8 9 10
  for (int i = 0; i < ROW; i++)
    for (int j = 0; j < COL; j++)
      ac[i][j] = vc++;

  mct_t bc = __builtin_matrix_transpose(ac);

  // CHECK:      5 8
  // CHECK-NEXT: 6 9
  // CHECK-NEXT: 7 10
  for (int i = 0; i < COL; i++) {
    for (int j = 0; j < ROW; j++)
      printf("%d ", bc[i][j]);
    printf("\n");
  }

  mui_t aui;
  unsigned int vui = 10;

  // Input matrix:
  // 10 11 12
  // 13 14 15
  for (int i = 0; i < ROW; i++)
    for (int j = 0; j < COL; j++)
      aui[i][j] = vui++;

  muit_t bui = __builtin_matrix_transpose(aui);

  // CHECK:      10 13
  // CHECK-NEXT: 11 14
  // CHECK-NEXT: 12 15
  for (int i = 0; i < COL; i++) {
    for (int j = 0; j < ROW; j++)
      printf("%d ", bui[i][j]);
    printf("\n");
  }

  return 0;
}
