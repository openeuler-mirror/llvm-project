; RUN: llc < %s --aarch64-prefetch-hints-file=%S/insert-prefetch.prof | FileCheck %s
;
; original source, compiled with -O3 -gmlt -fdebug-info-for-profiling:
; int sum(int *arr, int pos1, int po2) {
;   return arr[pos1] + arr[pos2];
; }
;
; ModuleID = 'sum.c'
source_filename = "sum.c"
target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable
define dso_local i32 @sum(ptr nocapture noundef readonly %arr, i32 noundef %pos1, i32 noundef %pos2) local_unnamed_addr #0 !dbg !10 {
entry:
  %idxprom = sext i32 %pos1 to i64, !dbg !13
  %arrayidx = getelementptr inbounds i32, ptr %arr, i64 %idxprom, !dbg !13
  %0 = load i32, ptr %arrayidx, align 4, !dbg !14, !tbaa !16
  %idxprom1 = sext i32 %pos2 to i64, !dbg !20
  %arrayidx2 = getelementptr inbounds i32, ptr %arr, i64 %idxprom1, !dbg !20
  %1 = load i32, ptr %arrayidx2, align 4, !dbg !21, !tbaa !16
  %add = add nsw i32 %1, %0, !dbg !23
  ret i32 %add, !dbg !24
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn memory(argmem: read) uwtable "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="generic" "target-features"="+fp-armv8,+neon,+v8a,-fmv" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C11, file: !1, producer: "clang version 17.0.6", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, debugInfoForProfiling: true, nameTableKind: None)
!1 = !DIFile(filename: "sum.c", directory: "/tmp")
!2 = !{i32 7, !"Dwarf Version", i32 4}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 8, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{i32 7, !"frame-pointer", i32 1}
!9 = !{!"clang version 17.0.6"}
!10 = distinct !DISubprogram(name: "sum", scope: !1, file: !1, line: 1, type: !11, scopeLine: 1, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0)
!11 = !DISubroutineType(types: !12)
!12 = !{}
!13 = !DILocation(line: 2, column: 10, scope: !10)
!14 = !DILocation(line: 2, column: 10, scope: !15)
!15 = !DILexicalBlockFile(scope: !10, file: !1, discriminator: 4)
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !DILocation(line: 2, column: 22, scope: !10)
!21 = !DILocation(line: 2, column: 22, scope: !22)
!22 = !DILexicalBlockFile(scope: !10, file: !1, discriminator: 10)
!23 = !DILocation(line: 2, column: 20, scope: !10)
!24 = !DILocation(line: 2, column: 3, scope: !10)

;CHECK-LABEL: sum:
;CHECK:       .loc  1 2 10 discriminator 4 // sum.c:2:10
;CHECK-NEXT:  prfm pldl1keep, [x8, #256]
;CHECK-NEXT:  ldr w8, [x8]
;CHECK-NEXT:  .loc  1 2 22 discriminator 10 // sum.c:2:22
;CHECK-NEXT:  prfm pldl1keep, [x9, #256]
;CHECK-NEXT:  ldr w9, [x9]
