; RUN: llc < %s | FileCheck %s
;
; NOTE: nc sched schdules load instructions together and store instructions
; together which exxcutes faster in non-cacheable memory.
;
; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable
define dso_local i32 @rdwr(i32 noundef %iter, ptr noundef %p, ptr noundef readnone %lastone) local_unnamed_addr #0 !dbg !10 {
entry:
  %cmp45 = icmp sgt i32 %iter, 0, !dbg !13
  br i1 %cmp45, label %while.cond1.preheader, label %while.end20, !dbg !15

while.cond.loopexit:                              ; preds = %while.body3, %while.cond1.preheader
  %p.addr.1.lcssa = phi ptr [ %p.addr.046, %while.cond1.preheader ], [ %add.ptr, %while.body3 ]
  %sum.1.lcssa = phi i32 [ %sum.047, %while.cond1.preheader ], [ %add18, %while.body3 ], !dbg !16
  %cmp = icmp sgt i32 %dec48.in, 1, !dbg !13
  br i1 %cmp, label %while.cond1.preheader, label %while.end20, !dbg !15, !llvm.loop !17

while.cond1.preheader:                            ; preds = %entry, %while.cond.loopexit
  %dec48.in = phi i32 [ %dec48, %while.cond.loopexit ], [ %iter, %entry ]
  %sum.047 = phi i32 [ %sum.1.lcssa, %while.cond.loopexit ], [ 0, %entry ]
  %p.addr.046 = phi ptr [ %p.addr.1.lcssa, %while.cond.loopexit ], [ %p, %entry ]
  %dec48 = add nsw i32 %dec48.in, -1, !dbg !21
  %cmp2.not40 = icmp ugt ptr %p.addr.046, %lastone, !dbg !22
  br i1 %cmp2.not40, label %while.cond.loopexit, label %while.body3, !dbg !23

while.body3:                                      ; preds = %while.cond1.preheader, %while.body3
  %sum.142 = phi i32 [ %add18, %while.body3 ], [ %sum.047, %while.cond1.preheader ]
  %p.addr.141 = phi ptr [ %add.ptr, %while.body3 ], [ %p.addr.046, %while.cond1.preheader ]
  %0 = load i32, ptr %p.addr.141, align 4, !dbg !24, !tbaa !25
  %add = add nsw i32 %0, %sum.142, !dbg !29
  store i32 1, ptr %p.addr.141, align 4, !dbg !30, !tbaa !25
  %arrayidx5 = getelementptr inbounds i32, ptr %p.addr.141, i64 4, !dbg !31
  %1 = load i32, ptr %arrayidx5, align 4, !dbg !31, !tbaa !25
  %add6 = add nsw i32 %add, %1, !dbg !32
  store i32 1, ptr %arrayidx5, align 4, !dbg !33, !tbaa !25
  %arrayidx8 = getelementptr inbounds i32, ptr %p.addr.141, i64 8, !dbg !34
  %2 = load i32, ptr %arrayidx8, align 4, !dbg !34, !tbaa !25
  %add9 = add nsw i32 %add6, %2, !dbg !35
  store i32 1, ptr %arrayidx8, align 4, !dbg !36, !tbaa !25
  %arrayidx11 = getelementptr inbounds i32, ptr %p.addr.141, i64 12, !dbg !37
  %3 = load i32, ptr %arrayidx11, align 4, !dbg !37, !tbaa !25
  %add12 = add nsw i32 %add9, %3, !dbg !38
  store i32 1, ptr %arrayidx11, align 4, !dbg !39, !tbaa !25
  %arrayidx14 = getelementptr inbounds i32, ptr %p.addr.141, i64 16, !dbg !40
  %4 = load i32, ptr %arrayidx14, align 4, !dbg !40, !tbaa !25
  %add15 = add nsw i32 %add12, %4, !dbg !41
  store i32 1, ptr %arrayidx14, align 4, !dbg !42, !tbaa !25
  %arrayidx17 = getelementptr inbounds i32, ptr %p.addr.141, i64 20, !dbg !43
  %5 = load i32, ptr %arrayidx17, align 4, !dbg !43, !tbaa !25
  %add18 = add nsw i32 %add15, %5, !dbg !44
  store i32 1, ptr %arrayidx17, align 4, !dbg !45, !tbaa !25
  %add.ptr = getelementptr inbounds i32, ptr %p.addr.141, i64 24, !dbg !46
  %cmp2.not = icmp ugt ptr %add.ptr, %lastone, !dbg !22
  br i1 %cmp2.not, label %while.cond.loopexit, label %while.body3, !dbg !23, !llvm.loop !47

while.end20:                                      ; preds = %while.cond.loopexit, %entry
  %sum.0.lcssa = phi i32 [ 0, %entry ], [ %sum.1.lcssa, %while.cond.loopexit ], !dbg !50
  ret i32 %sum.0.lcssa, !dbg !51
}

attributes #0 = { nofree norecurse nosync nounwind memory(readwrite, inaccessiblemem: none) uwtable "frame-pointer"="non-leaf" "ncsched" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fp-armv8,+neon,+v8a,-fmv" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 17.0.6", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, debugInfoForProfiling: true, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "/home/w00882830/llvm-project/llvm/test/CodeGen/AArch64")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 1}
!9 = !{!"clang version 17.0.6"}
!10 = distinct !DISubprogram(name: "rdwr", scope: !1, file: !1, line: 1, type: !11, scopeLine: 2, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!11 = !DISubroutineType(types: !12)
!12 = !{}
!13 = !DILocation(line: 4, column: 17, scope: !14)
!14 = !DILexicalBlockFile(scope: !10, file: !1, discriminator: 2)
!15 = !DILocation(line: 4, column: 3, scope: !14)
!16 = !DILocation(line: 0, scope: !10)
!17 = distinct !{!17, !18, !19, !20}
!18 = !DILocation(line: 4, column: 3, scope: !10)
!19 = !DILocation(line: 20, column: 3, scope: !10)
!20 = !{!"llvm.loop.mustprogress"}
!21 = !DILocation(line: 4, column: 14, scope: !14)
!22 = !DILocation(line: 5, column: 14, scope: !14)
!23 = !DILocation(line: 5, column: 5, scope: !14)
!24 = !DILocation(line: 6, column: 14, scope: !10)
!25 = !{!26, !26, i64 0}
!26 = !{!"int", !27, i64 0}
!27 = !{!"omnipotent char", !28, i64 0}
!28 = !{!"Simple C/C++ TBAA"}
!29 = !DILocation(line: 6, column: 11, scope: !10)
!30 = !DILocation(line: 7, column: 12, scope: !10)
!31 = !DILocation(line: 8, column: 14, scope: !10)
!32 = !DILocation(line: 8, column: 11, scope: !10)
!33 = !DILocation(line: 9, column: 12, scope: !10)
!34 = !DILocation(line: 10, column: 14, scope: !10)
!35 = !DILocation(line: 10, column: 11, scope: !10)
!36 = !DILocation(line: 11, column: 12, scope: !10)
!37 = !DILocation(line: 12, column: 14, scope: !10)
!38 = !DILocation(line: 12, column: 11, scope: !10)
!39 = !DILocation(line: 13, column: 13, scope: !10)
!40 = !DILocation(line: 14, column: 14, scope: !10)
!41 = !DILocation(line: 14, column: 11, scope: !10)
!42 = !DILocation(line: 15, column: 13, scope: !10)
!43 = !DILocation(line: 16, column: 14, scope: !10)
!44 = !DILocation(line: 16, column: 11, scope: !10)
!45 = !DILocation(line: 17, column: 13, scope: !10)
!46 = !DILocation(line: 18, column: 9, scope: !10)
!47 = distinct !{!47, !48, !49, !20}
!48 = !DILocation(line: 5, column: 5, scope: !10)
!49 = !DILocation(line: 19, column: 5, scope: !10)
!50 = !DILocation(line: 3, column: 16, scope: !10)
!51 = !DILocation(line: 21, column: 3, scope: !10)

; CHECK-LABEL: rdwr:
; CHECK: ldr w10, [x1]
; CHECK: ldr w11, [x1, #16]
; CHECK: ldr w12, [x1, #32]
; CHECK: ldr w13, [x1, #48]
; CHECK: ldr w14, [x1, #64]
; CHECK: ldr w15, [x1, #80]
; CHECK: str w9, [x1]
; CHECK: str w9, [x1, #16]
; CHECK: str w9, [x1, #32]
; CHECK: str w9, [x1, #48]
; CHECK: str w9, [x1, #64]
; CHECK: str w9, [x1, #80]
