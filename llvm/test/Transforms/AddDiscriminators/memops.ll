; RUN: opt < %s -passes=add-discriminators -S | FileCheck %s

; Check that -discriminate-memops is set by default.
; Check that discriminators are added for loads/stores on the same line:
;
; #1 int A, B, C;
; #2
; #3 void foo(int cond) {
; #4	 C /* discriminator 4 */ = cond ? A /* discriminator 0 */: B /* discriminator 2 */;
; #5 }

@A = dso_local local_unnamed_addr global i32 0, align 4
@B = dso_local local_unnamed_addr global i32 0, align 4
@C = dso_local local_unnamed_addr global i32 0, align 4

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn uwtable
define dso_local void @foo(i32 noundef %0) local_unnamed_addr #0 !dbg !9 {
  %2 = icmp eq i32 %0, 0, !dbg !12
  %3 = load i32, ptr @A, align 4, !dbg !12
  %4 = load i32, ptr @B, align 4, !dbg !12
  %5 = select i1 %2, i32 %4, i32 %3, !dbg !12
  store i32 %5, ptr @C, align 4, !dbg !13, !tbaa !14
  ret void, !dbg !18
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind willreturn uwtable "frame-pointer"="none" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+87" "tune-cpu"="generic" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 15.0.4", isOptimized: true, runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "memops.c", directory: "/srv/workspace/workspace-code/oh/tools/out/llvm_make", checksumkind: CSK_MD5, checksum: "5aa69f71aab38096119e5aab0df30495")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 7, !"PIC Level", i32 2}
!6 = !{i32 7, !"PIE Level", i32 2}
!7 = !{i32 7, !"uwtable", i32 2}
!8 = !{!"clang version 15.0.4"}
!9 = distinct !DISubprogram(name: "foo", file: !1, line: 3, type: !10, scopeLine: 3, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !11)
!10 = !DISubroutineType(types: !11)
!11 = !{}
!12 = !DILocation(line: 4, column: 31, scope: !9)
!13 = !DILocation(line: 4, column: 29, scope: !9)
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
!18 = !DILocation(line: 5, column: 1, scope: !9)

; CHECK: ![[SUBPROGRAM:[0-9]+]] = distinct !DISubprogram(name: "foo"
; CHECK: ![[LOADA:[0-9]+]] = !DILocation(line: 4, column: 31, scope: ![[SUBPROGRAM]])
; CHECK: ![[LOADB:[0-9]+]] = !DILocation(line: 4, column: 31, scope: ![[LOADBLOCK:[0-9]+]])
; CHECK: ![[LOADBLOCK]] = !DILexicalBlockFile(scope: ![[SUBPROGRAM]], {{.*}}discriminator: 2)
; CHECK: ![[STOREC:[0-9]+]] = !DILocation(line: 4, column: 29, scope: ![[STOREBLOCK:[0-9]+]])
; CHECK: ![[STOREBLOCK]] = !DILexicalBlockFile(scope: ![[SUBPROGRAM]], {{.*}}discriminator: 4)
