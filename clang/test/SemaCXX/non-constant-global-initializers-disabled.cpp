// RUN: %clang_cc1 -x c -fsyntax-only -fno-allow-non-const-global-init %s -verify
// RUN: %clang_cc1 -x c++ -fsyntax-only -fno-allow-non-const-global-init %s -verify

int* f(int *);
struct DT { int * el;};
int * pa = 0;
struct DT va = (struct DT){pa}; // expected-error {{initializer element is not a compile-time constant}}
struct DT vb = (struct DT){pa + 1}; // expected-error {{initializer element is not a compile-time constant}}
struct DT vc = (struct DT){f(pa) + 1}; // expected-error {{initializer element is not a compile-time constant}}
