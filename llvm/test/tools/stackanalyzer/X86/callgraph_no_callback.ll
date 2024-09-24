; REQUIRES: x86-registered-target
; RUN: llvm-as %s -o %t.bc
; RUN: stackanalyzer --callgraph %t.bc --entry=main --target=x86_64 | FileCheck %s --check-prefix=CHECK-MAIN
; RUN: stackanalyzer --callgraph %t.bc --entry=foo --target=x86_64 | FileCheck %s --check-prefix=CHECK-FOO
; RUN: stackanalyzer --callgraph %t.bc --entry=baz --target=x84_64 | FileCheck %s --check-prefix=CHECK-BAZ

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1

define dso_local i32 @baz(i32 noundef %0) #0 {
  %2 = alloca i32, align 4
  %3 = alloca [1024 x i8], align 16
  store i32 %0, ptr %2, align 4
  %4 = load i32, ptr %2, align 4
  %5 = mul nsw i32 %4, 3
  ret i32 %5
}

define dso_local i32 @foo(i32 noundef %0) #0 {
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

define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  store i32 0, ptr %1, align 4
  %3 = call i32 (ptr, ...) @__isoc99_scanf(ptr noundef @.str, ptr noundef %2)
  %4 = load i32, ptr %2, align 4
  %5 = call i32 @foo(i32 noundef %4)
  ret i32 0
}

declare i32 @__isoc99_scanf(ptr noundef, ...) #1

; CHECK-MAIN: Call graph node <<null function>>{{.*}}  #uses=0
; CHECK-MAIN:   CS<None> calls function 'baz'
; CHECK-MAIN:   CS<None> calls function 'foo'
; CHECK-MAIN:   CS<None> calls function 'main'
; CHECK-MAIN:   CS<None> calls function '__isoc99_scanf'

; CHECK-MAIN: Call graph node for function: '__isoc99_scanf'{{.*}}  #uses=2
; CHECK-MAIN:   CS<None> calls external node

; CHECK-MAIN: Call graph node for function: 'baz'{{.*}}  #uses=2

; CHECK-MAIN: Call graph node for function: 'foo'{{.*}}  #uses=2
; CHECK-MAIN:   CS{{.*}} calls function 'baz'

; CHECK-MAIN: Call graph node for function: 'main'{{.*}}  #uses=1
; CHECK-MAIN:   CS{{.*}} calls function '__isoc99_scanf'
; CHECK-MAIN:   CS{{.*}} calls function 'foo'

; CHECK-FOO: Call graph node <<null function>>{{.*}}  #uses=0
; CHECK-FOO:   CS<None> calls function 'baz'
; CHECK-FOO:   CS<None> calls function 'foo'
; CHECK-FOO:   CS<None> calls function 'main'
; CHECK-FOO:   CS<None> calls function '__isoc99_scanf'

; CHECK-FOO: Call graph node for function: '__isoc99_scanf'{{.*}}  #uses=2
; CHECK-FOO:   CS<None> calls external node

; CHECK-FOO: Call graph node for function: 'baz'{{.*}}  #uses=2

; CHECK-FOO: Call graph node for function: 'foo'{{.*}}  #uses=2
; CHECK-FOO:   CS{{.*}} calls function 'baz'

; CHECK-FOO: Call graph node for function: 'main'{{.*}}  #uses=1
; CHECK-FOO:   CS{{.*}} calls function '__isoc99_scanf'
; CHECK-FOO:   CS{{.*}} calls function 'foo'

; CHECK-BAZ: Call graph node <<null function>>{{.*}}  #uses=0
; CHECK-BAZ:   CS<None> calls function 'baz'
; CHECK-BAZ:   CS<None> calls function 'foo'
; CHECK-BAZ:   CS<None> calls function 'main'
; CHECK-BAZ:   CS<None> calls function '__isoc99_scanf'

; CHECK-BAZ: Call graph node for function: '__isoc99_scanf'{{.*}}  #uses=2
; CHECK-BAZ:   CS<None> calls external node

; CHECK-BAZ: Call graph node for function: 'baz'{{.*}}  #uses=2

; CHECK-BAZ: Call graph node for function: 'foo'{{.*}}  #uses=2
; CHECK-BAZ:   CS{{.*}} calls function 'baz'

; CHECK-BAZ: Call graph node for function: 'main'{{.*}}  #uses=1
; CHECK-BAZ:   CS{{.*}} calls function '__isoc99_scanf'
; CHECK-BAZ:   CS{{.*}} calls function 'foo'