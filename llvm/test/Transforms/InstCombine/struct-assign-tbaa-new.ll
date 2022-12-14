; RUN: opt -passes=instcombine -S < %s | FileCheck %s
;
; Verify that instcombine preserves TBAA tags when converting a memcpy into
; a scalar load and store.

target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"

declare void @llvm.memcpy.p0.p0.i64(ptr nocapture, ptr nocapture, i64, i1) nounwind

%A = type { float }

define void @test1(ptr %a1, ptr %a2) {
entry:
; CHECK-LABEL: @test1
; CHECK: %[[LOAD:.*]] = load i32, {{.*}}, !tbaa [[TAG_A:!.*]]
; CHECK: store i32 %[[LOAD]], {{.*}}, !tbaa [[TAG_A]]
; CHECK: ret
  tail call void @llvm.memcpy.p0.p0.i64(ptr align 4 %a1, ptr align 4 %a2, i64 4, i1 false), !tbaa !4  ; TAG_A
  ret void
}

%B = type { ptr }

define ptr @test2() {
; CHECK-LABEL: @test2
; CHECK-NOT: memcpy
; CHECK: ret
  %tmp = alloca %B, align 8
  call void @llvm.memcpy.p0.p0.i64(ptr align 8 %tmp, ptr align 8 undef, i64 8, i1 false), !tbaa !7  ; TAG_B
  %tmp3 = load ptr, ptr %tmp
  ret ptr %tmp
}

!0 = !{!"root"}
!1 = !{!0, !"char"}
!2 = !{!1, !"float"}
!3 = !{!1, i64 4, !"A", !2, i64 0, i64 4}
!4 = !{!3, !3, i64 0, i64 4}
!5 = !{!1, !"pointer"}
!6 = !{!1, i64 8, !"B", !5, i64 0, i64 8}
!7 = !{!6, !6, i64 0, i64 8}

; CHECK-DAG: [[ROOT:!.*]] = !{!"root"}
; CHECK-DAG: [[TYPE_char:!.*]] = !{[[ROOT]], !"char"}
; CHECK-DAG: [[TYPE_float:!.*]] = !{[[TYPE_char]], !"float"}
; CHECK-DAG: [[TYPE_A:!.*]] = !{[[TYPE_char]], i64 4, !"A", [[TYPE_float]], i64 0, i64 4}
; CHECK-DAG: [[TAG_A]] = !{[[TYPE_A]], [[TYPE_A]], i64 0, i64 4}
; Note that the memcpy() call in test2() transforms into an
; undecorated 'store undef', so TAG_B is not present in the output.
