; RUN: opt -passes=loop-vectorize -mtriple=aarch64-unknown-linux-gnu -mattr=+sve -S < %s | FileCheck %s --check-prefix=SVE
; RUN: opt -passes=loop-vectorize -mtriple=aarch64-unknown-linux-gnu -mattr=+sve -S < %s | FileCheck %s --check-prefix=NEON

define void @prefer_sve(ptr noalias nocapture %a, ptr noalias nocapture readonly %b, i64 %n) {
; SVE-LABEL: @prefer_sve(
; SVE: vector.body:
; SVE: load <vscale x
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %arrayidx.b = getelementptr inbounds i32, ptr %b, i64 %iv
  %val = load i32, ptr %arrayidx.b, align 4
  %add = add nsw i32 %val, 1
  %arrayidx.a = getelementptr inbounds i32, ptr %a, i64 %iv
  store i32 %add, ptr %arrayidx.a, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !0

for.end:
  ret void
}

define void @prefer_neon(ptr noalias nocapture %a, ptr noalias nocapture readonly %b, i64 %n) {
; NEON-LABEL: @prefer_neon(
; NEON: vector.body:
; NEON: load <{{[0-9]+}} x
; NEON-NOT: load <vscale x
; NEON: for.body:
entry:
  br label %for.body

for.body:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %for.body ]
  %arrayidx.b = getelementptr inbounds i32, ptr %b, i64 %iv
  %val = load i32, ptr %arrayidx.b, align 4
  %add = add nsw i32 %val, 1
  %arrayidx.a = getelementptr inbounds i32, ptr %a, i64 %iv
  store i32 %add, ptr %arrayidx.a, align 4
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, %n
  br i1 %exitcond, label %for.end, label %for.body, !llvm.loop !3

for.end:
  ret void
}

!0 = distinct !{!0, !1, !2}
!1 = !{!"llvm.loop.vectorize.enable", i1 true}
!2 = !{!"llvm.loop.vectorize.version", i32 1}
!3 = distinct !{!3, !1, !4}
!4 = !{!"llvm.loop.vectorize.version", i32 0}
