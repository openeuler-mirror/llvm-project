; RUN: llc -mtriple=aarch64-unknown -mcpu=hip09 -aarch64-sve-simplify-index-multiply -O3 -o - %s | FileCheck %s

define dso_local void @index_mul_simplify(i32 %loopTime, ptr %x, <vscale x 4 x float> %val) {
; CHECK-LABEL: index_mul_simplify:
; CHECK:       // %bb.1:                                // %for.body.lr.ph
; CHECK-DAG:   mov	w[[MULTIPLIER:[0-9]+]], #3
; CHECK-DAG:   index	z[[OFFSET_VEC:[0-9]+]].s, #0, #3
; CHECK-DAG:   cntw	x[[IV_STEP:[0-9]+]]
; CHECK-DAG:   cntw	x[[NEW_IV_STEP:[0-9]+]], all, mul #3
; CHECK-DAG:   mul	w[[NEW_IV_INIT:[0-9]+]], wzr, w[[MULTIPLIER]]

; CHECK:       .LBB0_2:                                // %for.body
; CHECK:       mov z[[BASE_VEC:[0-9]+]].s, w[[NEW_IV_CUR:[0-9]+]]
; CHECK:       whilelt	p[[PG:[0-9]+]].s, w{{[0-9]+}}, w{{[0-9]+}}
; CHECK:       add	w[[NEW_IV_CUR]], w[[NEW_IV_CUR]], w[[NEW_IV_STEP]]
; CHECK:       add	z[[FINAL_INDICES:[0-9]+]].s, z[[OFFSET_VEC]].s, z[[BASE_VEC]].s
; CHECK-NOT:   mul
; CHECK:       st1w	{ z0.s }, p[[PG]], [x1, z[[FINAL_INDICES]].s, sxtw #2]

entry:
  %cmp7 = icmp sgt i32 %loopTime, 0
  br i1 %cmp7, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %.tr = tail call i32 @llvm.vscale.i32()
  %0 = shl nuw nsw i32 %.tr, 2
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %jp.08 = phi i32 [ 0, %for.body.lr.ph ], [ %conv1, %for.body ]
  %1 = tail call <vscale x 4 x i1> @llvm.aarch64.sve.whilelt.nxv4i1.i32(i32 %jp.08, i32 %loopTime)
  %2 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.index.nxv4i32(i32 %jp.08, i32 1)
  %3 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %2, <vscale x 4 x i32> zeroinitializer
  %4 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.mul.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %3, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 3, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  tail call void @llvm.aarch64.sve.st1.scatter.sxtw.index.nxv4f32(<vscale x 4 x float> %val, <vscale x 4 x i1> %1, ptr %x, <vscale x 4 x i32> %4)
  %conv1 = add i32 %0, %jp.08
  %cmp = icmp slt i32 %conv1, %loopTime
  br i1 %cmp, label %for.body, label %for.cond.cleanup
}

declare <vscale x 4 x i1> @llvm.aarch64.sve.whilelt.nxv4i1.i32(i32, i32)
declare <vscale x 4 x i32> @llvm.aarch64.sve.index.nxv4i32(i32, i32)
declare <vscale x 4 x i32> @llvm.aarch64.sve.mul.nxv4i32(<vscale x 4 x i1>, <vscale x 4 x i32>, <vscale x 4 x i32>)
declare void @llvm.aarch64.sve.st1.scatter.sxtw.index.nxv4f32(<vscale x 4 x float>, <vscale x 4 x i1>, ptr, <vscale x 4 x i32>)
declare i32 @llvm.vscale.i32()