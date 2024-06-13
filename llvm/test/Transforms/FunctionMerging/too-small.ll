; RUN: opt -passes=func-merging -S < %s | FileCheck %s

; Too small for merging to be profitable

define void @foo(i32 %x) {
; CHECK-LABEL: @foo(
; CHECK-NOT: call
  ret void
}

define void @bar(i32 %x) {
; CHECK-LABEL: @bar(
; CHECK-NOT: call
  ret void
}

