; RUN: llvm-as %s -o %t.bc
; RUN: stackanalyzer --analysis %t.bc --entry=main --anders --target=x86_64 | FileCheck %s 

@.str = private unnamed_addr constant [5 x i8] c"%hhu\00", align 1

define dso_local i32 @foo(i8 noundef zeroext %0, ptr noundef %1) #0 {
  %3 = alloca i8, align 1
  %4 = alloca ptr, align 8
  %5 = alloca i8, align 1
  %6 = alloca [256 x i8], align 16
  store i8 %0, ptr %3, align 1
  store ptr %1, ptr %4, align 8
  %7 = load i8, ptr %5, align 1
  %8 = zext i8 %7 to i32
  %9 = call i32 @bar(i32 noundef %8)
  %10 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str, ptr noundef %5)
  %11 = load ptr, ptr %4, align 8
  %12 = load i8, ptr %5, align 1
  %13 = zext i8 %12 to i32
  %14 = call i32 %11(i32 noundef %13)
  ret i32 %14
}

declare i32 @__isoc99_scanf(ptr noundef, ...) #1

define dso_local i32 @baz(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca [1024 x i8], align 16
  store i32 %0, ptr %2, align 4
  %4 = load i32, ptr %2, align 4
  %5 = mul nsw i32 %4, 3
  ret i32 %5
}

define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i8, align 1
  %3 = alloca ptr, align 8
  store i32 0, ptr %1, align 4
  store ptr @baz, ptr %3, align 8
  %4 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str, ptr noundef %2)
  %5 = load i8, ptr %2, align 1
  %6 = load ptr, ptr %3, align 8
  %7 = call i32 @foo(i8 noundef zeroext %5, ptr noundef %6)
  ret i32 0
}

define dso_local i32 @bar(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  store i32 %0, ptr %2, align 4
  %4 = load i32, ptr %2, align 4
  store i32 %4, ptr %3, align 4
  %5 = load i32, ptr %3, align 4
  %6 = mul nsw i32 2, %5
  ret i32 %6
}

; CHECK: Potential stack overflow path found(limit:1024 bytes):
; CHECK-NEXT: CallStack:
; CHECK-NEXT:   main
; CHECK-NEXT:   foo
; CHECK-NEXT:   baz
; CHECK-NEXT: Analysis:
; CHECK-NEXT: - Stack usage exceeds the limit along the call stack.