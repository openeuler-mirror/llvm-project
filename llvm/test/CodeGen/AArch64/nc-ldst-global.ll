; RUN: llc < %s -force-group-ldst | FileCheck %s
; RUN: llc < %s | FileCheck %s --check-prefix=NOSCHED

; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-gnu"

; Function Attrs: nofree noinline norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable vscale_range(1,16)
define dso_local i32 @rdwr(i32 noundef %iter, ptr noundef %p, ptr noundef readnone %lastone) local_unnamed_addr #0 {
entry:
  %cmp45 = icmp sgt i32 %iter, 0
  br i1 %cmp45, label %while.cond1.preheader, label %while.end20

while.cond.loopexit:                              ; preds = %while.body3, %while.cond1.preheader
  %p.addr.1.lcssa = phi ptr [ %p.addr.046, %while.cond1.preheader ], [ %add.ptr, %while.body3 ]
  %sum.1.lcssa = phi i32 [ %sum.047, %while.cond1.preheader ], [ %add18, %while.body3 ]
  %cmp = icmp sgt i32 %dec48.in, 1
  br i1 %cmp, label %while.cond1.preheader, label %while.end20, !llvm.loop !6

while.cond1.preheader:                            ; preds = %entry, %while.cond.loopexit
  %dec48.in = phi i32 [ %dec48, %while.cond.loopexit ], [ %iter, %entry ]
  %sum.047 = phi i32 [ %sum.1.lcssa, %while.cond.loopexit ], [ 0, %entry ]
  %p.addr.046 = phi ptr [ %p.addr.1.lcssa, %while.cond.loopexit ], [ %p, %entry ]
  %dec48 = add nsw i32 %dec48.in, -1
  %cmp2.not40 = icmp ugt ptr %p.addr.046, %lastone
  br i1 %cmp2.not40, label %while.cond.loopexit, label %while.body3

while.body3:                                      ; preds = %while.cond1.preheader, %while.body3
  %sum.142 = phi i32 [ %add18, %while.body3 ], [ %sum.047, %while.cond1.preheader ]
  %p.addr.141 = phi ptr [ %add.ptr, %while.body3 ], [ %p.addr.046, %while.cond1.preheader ]
  %0 = load i32, ptr %p.addr.141, align 4, !tbaa !8
  %add = add nsw i32 %0, %sum.142
  store i32 1, ptr %p.addr.141, align 4, !tbaa !8
  %arrayidx5 = getelementptr inbounds i32, ptr %p.addr.141, i64 4
  %1 = load i32, ptr %arrayidx5, align 4, !tbaa !8
  %add6 = add nsw i32 %add, %1
  store i32 1, ptr %arrayidx5, align 4, !tbaa !8
  %arrayidx8 = getelementptr inbounds i32, ptr %p.addr.141, i64 8
  %2 = load i32, ptr %arrayidx8, align 4, !tbaa !8
  %add9 = add nsw i32 %add6, %2
  store i32 1, ptr %arrayidx8, align 4, !tbaa !8
  %arrayidx11 = getelementptr inbounds i32, ptr %p.addr.141, i64 12
  %3 = load i32, ptr %arrayidx11, align 4, !tbaa !8
  %add12 = add nsw i32 %add9, %3
  store i32 1, ptr %arrayidx11, align 4, !tbaa !8
  %arrayidx14 = getelementptr inbounds i32, ptr %p.addr.141, i64 16
  %4 = load i32, ptr %arrayidx14, align 4, !tbaa !8
  %add15 = add nsw i32 %add12, %4
  store i32 1, ptr %arrayidx14, align 4, !tbaa !8
  %arrayidx17 = getelementptr inbounds i32, ptr %p.addr.141, i64 20
  %5 = load i32, ptr %arrayidx17, align 4, !tbaa !8
  %add18 = add nsw i32 %add15, %5
  store i32 1, ptr %arrayidx17, align 4, !tbaa !8
  %add.ptr = getelementptr inbounds i32, ptr %p.addr.141, i64 24
  %cmp2.not = icmp ugt ptr %add.ptr, %lastone
  br i1 %cmp2.not, label %while.cond.loopexit, label %while.body3, !llvm.loop !12

while.end20:                                      ; preds = %while.cond.loopexit, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %sum.1.lcssa, %while.cond.loopexit ]
  ret i32 %sum.0.lcssa
}

attributes #0 = { nofree noinline norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable vscale_range(1,16) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="hip09" "target-features"="+aes,+bf16,+crc,+dotprod,+f32mm,+f64mm,+fp-armv8,+fp16fml,+fullfp16,+i8mm,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+spe,+sve,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,-fmv" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"clang version 17.0.6"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = !{!9, !9, i64 0}
!9 = !{!"int", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C/C++ TBAA"}
!12 = distinct !{!12, !7}

; CHECK-LABEL: rdwr:
; CHECK: ldr w10, [x1]
; CHECK: ldr w11, [x1, #16]
; CHECK: ldr w10, [x1, #32]
; CHECK: ldr w11, [x1, #48]
; CHECK: ldr w10, [x1, #64]
; CHECK: ldr w11, [x1, #80]
; CHECK: str w9, [x1]
; CHECK: str w9, [x1, #16]
; CHECK: str w9, [x1, #32]
; CHECK: str w9, [x1, #48]
; CHECK: str w9, [x1, #64]
; CHECK: str w9, [x1, #80]

; NOSCHED-LABEL: rdwr:
; NOSCHED: ldr w10, [x1]
; NOSCHED: ldr w11, [x1, #16]
; NOSCHED: str w9, [x1]
; NOSCHED: str w9, [x1, #16]
; NOSCHED: ldr w10, [x1, #32]
; NOSCHED: str w9, [x1, #32]
; NOSCHED: ldr w11, [x1, #48]
; NOSCHED: str w9, [x1, #48]
; NOSCHED: ldr w10, [x1, #64]
; NOSCHED: str w9, [x1, #64]
; NOSCHED: ldr w11, [x1, #80]
; NOSCHED: str w9, [x1, #80]
