// RUN: %clang_cc1 %s -no-enable-noundef-analysis -triple i686-apple-darwin -emit-llvm -o - | FileCheck %s
// Every printf has 'i32 0' for the GEP of the string; no point counting those.
typedef unsigned int Foo __attribute__((aligned(32)));
typedef union{Foo:0;}a;
typedef union{int x; Foo:0;}b;
extern int printf(const char*, ...);
int main(void) {
  // CHECK: @.str, i32 0
  printf("%ld\n", sizeof(a));
  // CHECK: @.str, i32 1
  printf("%ld\n", __alignof__(a));
  // CHECK: @.str, i32 4
  printf("%ld\n", sizeof(b));
  // CHECK: @.str, i32 4
  printf("%ld\n", __alignof__(b));
}
