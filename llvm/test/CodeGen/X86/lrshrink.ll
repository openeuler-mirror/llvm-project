; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=x86_64-unknown-unknown | FileCheck %s

; Checks if "%7 = add nuw nsw i64 %4, %2" is moved before the last call
; to minimize live-range.

define i64 @test(i1 %a, i64 %r1, i64 %r2, i64 %s1, i64 %s2, i64 %t1, i64 %t2) {
; CHECK-LABEL: test:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    pushq %r15
; CHECK-NEXT:    .cfi_def_cfa_offset 16
; CHECK-NEXT:    pushq %r14
; CHECK-NEXT:    .cfi_def_cfa_offset 24
; CHECK-NEXT:    pushq %rbx
; CHECK-NEXT:    .cfi_def_cfa_offset 32
; CHECK-NEXT:    .cfi_offset %rbx, -32
; CHECK-NEXT:    .cfi_offset %r14, -24
; CHECK-NEXT:    .cfi_offset %r15, -16
; CHECK-NEXT:    movq %rcx, %rbx
; CHECK-NEXT:    movl $4, %r14d
; CHECK-NEXT:    testb $1, %dil
; CHECK-NEXT:    je .LBB0_2
; CHECK-NEXT:  # %bb.1: # %then
; CHECK-NEXT:    movq {{[0-9]+}}(%rsp), %r9
; CHECK-NEXT:    movl $10, %r14d
; CHECK-NEXT:    movq %rdx, %rsi
; CHECK-NEXT:    movq %r8, %rbx
; CHECK-NEXT:  .LBB0_2: # %else
; CHECK-NEXT:    addq %r9, %rbx
; CHECK-NEXT:    addq %rsi, %r14
; CHECK-NEXT:    callq _Z3foov@PLT
; CHECK-NEXT:    movl %eax, %r15d
; CHECK-NEXT:    addq %r14, %r15
; CHECK-NEXT:    callq _Z3foov@PLT
; CHECK-NEXT:    movl %eax, %r14d
; CHECK-NEXT:    addq %r15, %r14
; CHECK-NEXT:    callq _Z3foov@PLT
; CHECK-NEXT:    movl %eax, %eax
; CHECK-NEXT:    addq %r14, %rax
; CHECK-NEXT:    addq %rbx, %rax
; CHECK-NEXT:    popq %rbx
; CHECK-NEXT:    .cfi_def_cfa_offset 24
; CHECK-NEXT:    popq %r14
; CHECK-NEXT:    .cfi_def_cfa_offset 16
; CHECK-NEXT:    popq %r15
; CHECK-NEXT:    .cfi_def_cfa_offset 8
; CHECK-NEXT:    retq
entry:
  br i1 %a, label %then, label %else

then:
  br label %else

else:
  %0 = phi i64 [ 4, %entry ], [ 10, %then ]
  %r = phi i64 [ %r1, %entry ], [ %r2, %then ]
  %s = phi i64 [ %s1, %entry ], [ %s2, %then ]
  %t = phi i64 [ %t1, %entry ], [ %t2, %then ]
  %1 = tail call i32 @_Z3foov()
  %2 = zext i32 %1 to i64
  %3 = tail call i32 @_Z3foov()
  %4 = zext i32 %3 to i64
  %5 = tail call i32 @_Z3foov()
  %6 = zext i32 %5 to i64
  %7 = add nuw nsw i64 %0, %r
  tail call void @llvm.dbg.value(metadata i64 %7, i64 0, metadata !5, metadata !DIExpression()), !dbg !6
  %8 = add nuw nsw i64 %2, %7
  %9 = add nuw nsw i64 %4, %8
  %10 = add nuw nsw i64 %6, %9
  %11 = add nuw nsw i64 %s, %t
  tail call void @llvm.dbg.value(metadata i64 %11, i64 0, metadata !5, metadata !DIExpression()), !dbg !6
  %12 = add nuw nsw i64 %10, %11
  ret i64 %12
}

declare i32 @_Z3foov()
declare void @llvm.dbg.value(metadata, i64, metadata, metadata)

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !3, emissionKind: FullDebug)
!1 = !{i32 2, !"Dwarf Version", i32 4}
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !DIFile(filename: "a.c", directory: "./")
!4 = distinct !DISubprogram(name: "test", scope: !3, unit: !0)
!5 = !DILocalVariable(name: "x", scope: !4)
!6 = !DILocation(line: 4, scope: !4)
