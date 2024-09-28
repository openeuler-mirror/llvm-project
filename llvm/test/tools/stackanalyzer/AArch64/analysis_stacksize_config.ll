; REQUIRES: aarch64-registered-target
; RUN: llvm-as %s -o %t.bc
; RUN: stackanalyzer --analysis %t.bc --entry=main --stacksize=2048 --target=aarch64 | FileCheck %s --check-prefix=CHECK-2048
; RUN: stackanalyzer --analysis %t.bc --entry=main --stacksize=512 --target=aarch64 | FileCheck %s --check-prefix=CHECK-512

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1

define dso_local i32 @baz(i32 noundef %0) {
  %2 = alloca i32, align 4
  %3 = alloca [1024 x i8], align 16
  store i32 %0, ptr %2, align 4
  %4 = load i32, ptr %2, align 4
  %5 = mul nsw i32 %4, 3
  ret i32 %5
}

define dso_local i32 @foo(i32 noundef %0) {
  %2 = alloca i32, align 4
  %3 = alloca [256 x i8], align 16
  %4 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %5 = load i32, ptr %2, align 4
  %6 = call i32 @baz(i32 noundef %5)
  store i32 %6, ptr %4, align 4
  %7 = load i32, ptr %4, align 4
  ret i32 %7
}

define dso_local i32 @main() {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  %3 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str, ptr noundef %2)
  %4 = load i32, ptr %2, align 4
  %5 = call i32 @foo(i32 noundef %4)
  ret i32 0
}

declare i32 @__isoc99_scanf(ptr noundef, ...)

; CHECK-2048: No potential stack overflow path found(limit:2048 bytes).

; CHECK-512: Potential stack overflow path found(limit:512 bytes):
; CHECK-512-NEXT: CallStack:
; CHECK-512-NEXT:   main: 16 bytes.
; CHECK-512-NEXT:   foo: 304 bytes.
; CHECK-512-NEXT:   baz: 1040 bytes.
; CHECK-512-NEXT: Analysis:
; CHECK-512-NEXT: - Stack usage exceeds the limit along the call stack.
; CHECK-512-NEXT: - Total stack usage: 1360 bytes.
