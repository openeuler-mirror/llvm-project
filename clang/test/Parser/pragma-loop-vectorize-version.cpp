// RUN: %clang_cc1 -std=c++11 -verify %s

void test(int *List, int Length) {
#pragma clang loop vectorize_version(sve)
  for (int i = 0; i < Length; i++)
    List[i] = i;

#pragma clang loop vectorize_width(4) vectorize_version(neon)
  for (int i = 0; i < Length; i++)
    List[i] = i;

/* expected-error {{invalid argument; expected 'sve' or 'neon'}} */ #pragma clang loop vectorize_version()
  for (int i = 0; i < Length; i++)
    List[i] = i;

/* expected-error {{invalid argument; expected 'sve' or 'neon'}} */ #pragma clang loop vectorize_version(enable)
  for (int i = 0; i < Length; i++)
    List[i] = i;

/* expected-error {{invalid argument; expected 'sve' or 'neon'}} */ #pragma clang loop vectorize_version(disable)
  for (int i = 0; i < Length; i++)
    List[i] = i;

#pragma clang loop vectorize_version(sve)
/* expected-error {{duplicate directives 'vectorize_version(sve)' and 'vectorize_version(neon)'}} */ #pragma clang loop vectorize_version(neon)
  for (int i = 0; i < Length; i++)
    List[i] = i;
}
