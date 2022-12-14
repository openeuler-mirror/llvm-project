; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt -debugify -loop-idiom -pass-remarks=loop-idiom -pass-remarks-analysis=loop-idiom -pass-remarks-output=%t.yaml -verify -verify-each -verify-dom-info -verify-loop-info  < %s -S 2>&1 | FileCheck %s
; RUN: FileCheck --input-file=%t.yaml %s --check-prefixes=YAML

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Check that everything still works when debuginfo is present, and that it is reasonably propagated.

; CHECK: remark: <stdin>:6:1: Formed a call to llvm.memcpy.p0i8.p0i8.i64() intrinsic from load and store instruction in test6_dest_align function{{$}}

; YAML:      --- !Passed
; YAML-NEXT: Pass:            loop-idiom
; YAML-NEXT: Name:            ProcessLoopStoreOfLoopLoad
; YAML-NEXT: DebugLoc:        { File: '<stdin>', Line: 6, Column: 1 }
; YAML-NEXT: Function:        test6_dest_align
; YAML-NEXT: Args:
; YAML-NEXT:   - String:          'Formed a call to '
; YAML-NEXT:   - NewFunction:     llvm.memcpy.p0i8.p0i8.i64
; YAML-NEXT:   - String:          '() intrinsic from '
; YAML-NEXT:   - Inst:            load and store
; YAML-NEXT:   - String:          ' instruction in '
; YAML-NEXT:   - Function:        test6_dest_align
; YAML-NEXT:     DebugLoc:        { File: '<stdin>', Line: 1, Column: 0 }
; YAML-NEXT:   - String:          ' function'
; YAML-NEXT:   - FromBlock:       for.body
; YAML-NEXT:   - ToBlock:         bb.nph
; YAML-NEXT: ...

define void @test6_dest_align(i32* noalias align 1 %Base, i32* noalias align 4 %Dest, i64 %Size) nounwind ssp {
; CHECK-LABEL: @test6_dest_align(
; CHECK-NEXT:  bb.nph:
; CHECK-NEXT:    [[DEST1:%.*]] = bitcast i32* [[DEST:%.*]] to i8*, !dbg [[DBG18:![0-9]+]]
; CHECK-NEXT:    [[BASE2:%.*]] = bitcast i32* [[BASE:%.*]] to i8*, !dbg [[DBG18]]
; CHECK-NEXT:    [[TMP0:%.*]] = shl nuw i64 [[SIZE:%.*]], 2, !dbg [[DBG18]]
; CHECK-NEXT:    call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 [[DEST1]], i8* align 1 [[BASE2]], i64 [[TMP0]], i1 false), !dbg [[DBG19:![0-9]+]]
; CHECK-NEXT:    br label [[FOR_BODY:%.*]], !dbg [[DBG18]]
; CHECK:       for.body:
; CHECK-NEXT:    [[INDVAR:%.*]] = phi i64 [ 0, [[BB_NPH:%.*]] ], [ [[INDVAR_NEXT:%.*]], [[FOR_BODY]] ], !dbg [[DBG20:![0-9]+]]
; CHECK-NEXT:    call void @llvm.dbg.value(metadata i64 [[INDVAR]], metadata [[META9:![0-9]+]], metadata !DIExpression()), !dbg [[DBG20]]
; CHECK-NEXT:    [[I_0_014:%.*]] = getelementptr i32, i32* [[BASE]], i64 [[INDVAR]], !dbg [[DBG21:![0-9]+]]
; CHECK-NEXT:    call void @llvm.dbg.value(metadata i32* [[I_0_014]], metadata [[META11:![0-9]+]], metadata !DIExpression()), !dbg [[DBG21]]
; CHECK-NEXT:    [[DESTI:%.*]] = getelementptr i32, i32* [[DEST]], i64 [[INDVAR]], !dbg [[DBG22:![0-9]+]]
; CHECK-NEXT:    call void @llvm.dbg.value(metadata i32* [[DESTI]], metadata [[META12:![0-9]+]], metadata !DIExpression()), !dbg [[DBG22]]
; CHECK-NEXT:    [[V:%.*]] = load i32, i32* [[I_0_014]], align 1, !dbg [[DBG23:![0-9]+]]
; CHECK-NEXT:    call void @llvm.dbg.value(metadata i32 [[V]], metadata [[META13:![0-9]+]], metadata !DIExpression()), !dbg [[DBG23]]
; CHECK-NEXT:    [[INDVAR_NEXT]] = add i64 [[INDVAR]], 1, !dbg [[DBG24:![0-9]+]]
; CHECK-NEXT:    call void @llvm.dbg.value(metadata i64 [[INDVAR_NEXT]], metadata [[META15:![0-9]+]], metadata !DIExpression()), !dbg [[DBG24]]
; CHECK-NEXT:    [[EXITCOND:%.*]] = icmp eq i64 [[INDVAR_NEXT]], [[SIZE]], !dbg [[DBG25:![0-9]+]]
; CHECK-NEXT:    call void @llvm.dbg.value(metadata i1 [[EXITCOND]], metadata [[META16:![0-9]+]], metadata !DIExpression()), !dbg [[DBG25]]
; CHECK-NEXT:    br i1 [[EXITCOND]], label [[FOR_END:%.*]], label [[FOR_BODY]], !dbg [[DBG26:![0-9]+]]
; CHECK:       for.end:
; CHECK-NEXT:    ret void, !dbg [[DBG27:![0-9]+]]
;
bb.nph:
  br label %for.body

for.body:                                         ; preds = %bb.nph, %for.body
  %indvar = phi i64 [ 0, %bb.nph ], [ %indvar.next, %for.body ]
  %I.0.014 = getelementptr i32, i32* %Base, i64 %indvar
  %DestI = getelementptr i32, i32* %Dest, i64 %indvar
  %V = load i32, i32* %I.0.014, align 1
  store i32 %V, i32* %DestI, align 4
  %indvar.next = add i64 %indvar, 1
  %exitcond = icmp eq i64 %indvar.next, %Size
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body, %entry
  ret void
}
