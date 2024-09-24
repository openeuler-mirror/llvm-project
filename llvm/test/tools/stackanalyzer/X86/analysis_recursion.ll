; RUN: llvm-as %s -o %t.bc
; RUN: stackanalyzer --analysis %t.bc --entry=recursive_function --target=x86_64 | FileCheck %s

define void @recursive_function(i32 %n) {
  %cmp = icmp eq i32 %n, 0
  br i1 %cmp, label %base_case, label %recursive_case

base_case:
  ret void

recursive_case:
  %dec = sub i32 %n, 1
  call void @recursive_function(i32 %dec)
  ret void
}

; CHECK: Potential stack overflow path found(limit:1024 bytes):
; CHECK-NEXT: CallStack:
; CHECK-NEXT:   recursive_function
; CHECK-NEXT:   recursive_function
; CHECK-NEXT: Analysis:
; CHECK-NEXT: - Recursive call without proper base case check.
; CHECK-NEXT: - Unbounded recursion may lead to stack overflow.