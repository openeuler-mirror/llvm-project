; REQUIRES: enable_enable_aarch64_hip09
; RUN: llc -mtriple=aarch64-linux-gnuabi -mcpu=hip09 -o - %s | FileCheck %s

%X = type { i64, i64, i64 }
declare void @f(ptr)
define void @t() {
entry:
  %tmp = alloca %X
  call void @f(ptr %tmp)
; CHECK: add x0, sp, #8
; CHECK-NOT: mov
; CHECK-NEXT: bl f
  call void @f(ptr %tmp)
; CHECK: add x0, sp, #8
; CHECK-NOT: mov
; CHECK-NEXT: bl f
  ret void
}
