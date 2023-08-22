// The first and second runs check that nothing has been changed for C
// RUN: not %clang_cc1 -x c -fsyntax-only %s 2> %t
// RUN: FileCheck %s < %t
// RUN: not %clang_cc1 -x c -fsyntax-only -fallow-non-const-global-init %s 2> %t
// RUN: FileCheck %s < %t

// The third and fourth runs check that it works for C++ both by default and by enabling the option
// RUN: %clang_cc1 -x c++ -fsyntax-only %s -verify
// RUN: %clang_cc1 -x c++ -fsyntax-only -fallow-non-const-global-init %s -verify
// expected-no-diagnostics

int* f(int *);
struct DT { int * el;};
int * pa = 0;
struct DT va = (struct DT){pa};
// CHECK:[[@LINE-1]]:28: error: initializer element is not a compile-time constant
struct DT vb = (struct DT){pa + 1};
// CHECK:[[@LINE-1]]:31: error: initializer element is not a compile-time constant
struct DT vc = (struct DT){f(pa) + 1};
// CHECK:[[@LINE-1]]:34: error: initializer element is not a compile-time constant
