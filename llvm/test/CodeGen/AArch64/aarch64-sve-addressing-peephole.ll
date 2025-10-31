; RUN: llc -mtriple=aarch64-unknown -mcpu=hip09 -aarch64-sve-loop-address-chain-opt -O3 %s -o - | FileCheck %s

define dso_local void @test_gather_multi_constOffset(i32 noundef %loopTime, ptr noundef %x, float noundef %ipx, float noundef %ipy, float noundef %ipz, ptr nocapture noundef nonnull align 4 dereferenceable(4) %tempx, ptr nocapture noundef nonnull align 4 dereferenceable(4) %tempy, ptr nocapture noundef nonnull align 4 dereferenceable(4) %tempz) local_unnamed_addr #0 {
; CHECK-LABEL: test_gather_multi_constOffset:
; CHECK:       .LBB0_2:                                // %for.body
; CHECK:       add	x[[NEWBASE1:[0-9]+]], x1, #4
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG:[0-9]+]]/z, [x1, z{{[0-9]+}}.s, sxtw #2]
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG]]/z, [x[[NEWBASE1]], z{{[0-9]+}}.s, sxtw #2]
; CHECK:       add	x[[NEWBASE2:[0-9]+]], x1, #8
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG]]/z, [x[[NEWBASE2]], z{{[0-9]+}}.s, sxtw #2]
entry:
  %cmp18 = icmp sgt i32 %loopTime, 0
  br i1 %cmp18, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %.splatinsert = insertelement <vscale x 4 x float> poison, float %ipx, i64 0
  %.splat = shufflevector <vscale x 4 x float> %.splatinsert, <vscale x 4 x float> poison, <vscale x 4 x i32> zeroinitializer
  %.splatinsert2 = insertelement <vscale x 4 x float> poison, float %ipy, i64 0
  %.splat3 = shufflevector <vscale x 4 x float> %.splatinsert2, <vscale x 4 x float> poison, <vscale x 4 x i32> zeroinitializer
  %.splatinsert5 = insertelement <vscale x 4 x float> poison, float %ipz, i64 0
  %.splat6 = shufflevector <vscale x 4 x float> %.splatinsert5, <vscale x 4 x float> poison, <vscale x 4 x i32> zeroinitializer
  %.tr = tail call i32 @llvm.vscale.i32()
  %0 = shl nuw nsw i32 %.tr, 2
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %jp.019 = phi i32 [ 0, %for.body.lr.ph ], [ %conv10, %for.body ]
  %1 = tail call <vscale x 4 x i1> @llvm.aarch64.sve.whilelt.nxv4i1.i32(i32 %jp.019, i32 %loopTime)
  %2 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.index.nxv4i32(i32 %jp.019, i32 1)
  %3 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %2, <vscale x 4 x i32> zeroinitializer
  %4 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.mul.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %3, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 3, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %5 = tail call <vscale x 4 x float> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4f32(<vscale x 4 x i1> %1, ptr %x, <vscale x 4 x i32> %4)
  %6 = select <vscale x 4 x i1> %1, <vscale x 4 x float> %5, <vscale x 4 x float> zeroinitializer
  %7 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fsubr.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %6, <vscale x 4 x float> %.splat)
  %8 = select <vscale x 4 x i1> %1, <vscale x 4 x float> %7, <vscale x 4 x float> zeroinitializer
  %9 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fmul.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %8, <vscale x 4 x float> %7)
  %10 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %4, <vscale x 4 x i32> zeroinitializer
  %11 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %10, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 1, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %12 = tail call <vscale x 4 x float> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4f32(<vscale x 4 x i1> %1, ptr %x, <vscale x 4 x i32> %11)
  %13 = select <vscale x 4 x i1> %1, <vscale x 4 x float> %12, <vscale x 4 x float> zeroinitializer
  %14 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fsub.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %13, <vscale x 4 x float> %.splat3)
  %15 = select <vscale x 4 x i1> %1, <vscale x 4 x float> %14, <vscale x 4 x float> zeroinitializer
  %16 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fmad.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %15, <vscale x 4 x float> %14, <vscale x 4 x float> %9)
  %17 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %11, <vscale x 4 x i32> zeroinitializer
  %18 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %17, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 1, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %19 = tail call <vscale x 4 x float> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4f32(<vscale x 4 x i1> %1, ptr %x, <vscale x 4 x i32> %18)
  %20 = select <vscale x 4 x i1> %1, <vscale x 4 x float> %19, <vscale x 4 x float> zeroinitializer
  %21 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fsub.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %20, <vscale x 4 x float> %.splat6)
  %22 = select <vscale x 4 x i1> %1, <vscale x 4 x float> %21, <vscale x 4 x float> zeroinitializer
  %23 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fmad.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %22, <vscale x 4 x float> %21, <vscale x 4 x float> %16)
  %24 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fsqrt.nxv4f32(<vscale x 4 x float> zeroinitializer, <vscale x 4 x i1> %1, <vscale x 4 x float> %23)
  %25 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fmul.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %8, <vscale x 4 x float> %24)
  %26 = tail call float @llvm.aarch64.sve.faddv.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %25)
  %27 = load float, ptr %tempx, align 4, !tbaa !5
  %add = fadd float %26, %27
  store float %add, ptr %tempx, align 4, !tbaa !5
  %28 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fmul.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %15, <vscale x 4 x float> %24)
  %29 = tail call float @llvm.aarch64.sve.faddv.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %28)
  %30 = load float, ptr %tempy, align 4, !tbaa !5
  %add7 = fadd float %29, %30
  store float %add7, ptr %tempy, align 4, !tbaa !5
  %31 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fmul.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %22, <vscale x 4 x float> %24)
  %32 = tail call float @llvm.aarch64.sve.faddv.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %31)
  %33 = load float, ptr %tempz, align 4, !tbaa !5
  %add8 = fadd float %32, %33
  store float %add8, ptr %tempz, align 4, !tbaa !5
  %conv10 = add i32 %0, %jp.019
  %cmp = icmp slt i32 %conv10, %loopTime
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !llvm.loop !9
}

define dso_local void @test_scatter_constOffset(i32 noundef %loopTime, ptr noalias noundef %dst, ptr noalias nocapture noundef readonly %tempx, ptr noalias nocapture noundef readonly %tempy, ptr noalias nocapture noundef readonly %tempz) local_unnamed_addr #0 {
; CHECK-LABEL: test_scatter_constOffset:
; CHECK:       .LBB1_2:                                // %for.body
; CHECK:       add	x[[NEWBASE1:[0-9]+]], x1, #4
; CHECK:       st1w	{ z{{[0-9]+}}.s }, p[[PG:[0-9]+]], [x1, z{{[0-9]+}}.s, sxtw #2]
; CHECK:       st1w	{ z{{[0-9]+}}.s }, p[[PG]], [x[[NEWBASE1]], z{{[0-9]+}}.s, sxtw #2]
; CHECK:       add	x[[NEWBASE2:[0-9]+]], x1, #8
; CHECK:       st1w	{ z{{[0-9]+}}.s }, p[[PG]], [x[[NEWBASE2]], z{{[0-9]+}}.s, sxtw #2]
entry:
  %cmp15 = icmp sgt i32 %loopTime, 0
  br i1 %cmp15, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %.tr = tail call i32 @llvm.vscale.i32()
  %0 = shl nuw nsw i32 %.tr, 2
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %jp.016 = phi i32 [ 0, %for.body.lr.ph ], [ %conv5, %for.body ]
  %1 = tail call <vscale x 4 x i1> @llvm.aarch64.sve.whilelt.nxv4i1.i32(i32 %jp.016, i32 %loopTime)
  %2 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.index.nxv4i32(i32 %jp.016, i32 1)
  %3 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %2, <vscale x 4 x i32> zeroinitializer
  %4 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.mul.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %3, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 3, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %5 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %4, <vscale x 4 x i32> zeroinitializer
  %6 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %5, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 1, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %7 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %5, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 2, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %idx.ext = sext i32 %jp.016 to i64
  %add.ptr = getelementptr inbounds float, ptr %tempx, i64 %idx.ext
  %8 = tail call <vscale x 4 x float> @llvm.masked.load.nxv4f32.p0(ptr %add.ptr, i32 1, <vscale x 4 x i1> %1, <vscale x 4 x float> zeroinitializer), !tbaa !5
  %add.ptr2 = getelementptr inbounds float, ptr %tempy, i64 %idx.ext
  %9 = tail call <vscale x 4 x float> @llvm.masked.load.nxv4f32.p0(ptr %add.ptr2, i32 1, <vscale x 4 x i1> %1, <vscale x 4 x float> zeroinitializer), !tbaa !5
  %add.ptr4 = getelementptr inbounds float, ptr %tempz, i64 %idx.ext
  %10 = tail call <vscale x 4 x float> @llvm.masked.load.nxv4f32.p0(ptr %add.ptr4, i32 1, <vscale x 4 x i1> %1, <vscale x 4 x float> zeroinitializer), !tbaa !5
  tail call void @llvm.aarch64.sve.st1.scatter.sxtw.index.nxv4f32(<vscale x 4 x float> %8, <vscale x 4 x i1> %1, ptr %dst, <vscale x 4 x i32> %4)
  tail call void @llvm.aarch64.sve.st1.scatter.sxtw.index.nxv4f32(<vscale x 4 x float> %9, <vscale x 4 x i1> %1, ptr %dst, <vscale x 4 x i32> %6)
  tail call void @llvm.aarch64.sve.st1.scatter.sxtw.index.nxv4f32(<vscale x 4 x float> %10, <vscale x 4 x i1> %1, ptr %dst, <vscale x 4 x i32> %7)
  %conv5 = add i32 %0, %jp.016
  %cmp = icmp slt i32 %conv5, %loopTime
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !llvm.loop !11
}

define dso_local void @test_prefetch_constOffset(i32 noundef %loopTime, ptr nocapture noundef %data) local_unnamed_addr #4 {
; CHECK-LABEL: test_prefetch_constOffset:
; CHECK:       // %bb.4:                               // %if.end
; CHECK:       add	x[[NEWBASE1:[0-9]+]], x1, #4
; CHECK:       prfw	pldl1keep, p[[PG:[0-9]+]], [x1, z{{[0-9]+}}.s, sxtw #2]
; CHECK:       prfw	pldl1keep, p[[PG]], [x[[NEWBASE1]], z{{[0-9]+}}.s, sxtw #2]
; CHECK:       add	x[[NEWBASE2:[0-9]+]], x1, #8
; CHECK:       prfw	pldl1keep, p[[PG]], [x[[NEWBASE2]], z{{[0-9]+}}.s, sxtw #2]
entry:
  %.tr = tail call i32 @llvm.vscale.i32()
  %conv = shl nuw nsw i32 %.tr, 2
  %cmp13 = icmp sgt i32 %loopTime, 0
  br i1 %cmp13, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %0 = tail call <vscale x 4 x i1> @llvm.aarch64.sve.ptrue.nxv4i1(i32 31)
  br label %for.body

for.cond.cleanup:                                 ; preds = %cleanup, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %cleanup
  %jp.014 = phi i32 [ 0, %for.body.lr.ph ], [ %add, %cleanup ]
  %add = add i32 %jp.014, %conv
  %1 = tail call <vscale x 4 x i1> @llvm.aarch64.sve.whilelt.nxv4i1.i32(i32 %add, i32 %loopTime)
  %2 = tail call i1 @llvm.aarch64.sve.ptest.any.nxv4i1(<vscale x 4 x i1> %0, <vscale x 4 x i1> %1)
  br i1 %2, label %if.end, label %cleanup

if.end:                                           ; preds = %for.body
  %3 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.index.nxv4i32(i32 %add, i32 1)
  %4 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %3, <vscale x 4 x i32> zeroinitializer
  %5 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.mul.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %4, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 3, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  tail call void @llvm.aarch64.sve.prfw.gather.sxtw.index.nxv4i32(<vscale x 4 x i1> %1, ptr %data, <vscale x 4 x i32> %5, i32 0)
  %6 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %5, <vscale x 4 x i32> zeroinitializer
  %7 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %6, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 1, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  tail call void @llvm.aarch64.sve.prfw.gather.sxtw.index.nxv4i32(<vscale x 4 x i1> %1, ptr %data, <vscale x 4 x i32> %7, i32 0)
  %8 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %7, <vscale x 4 x i32> zeroinitializer
  %9 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %8, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 1, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  tail call void @llvm.aarch64.sve.prfw.gather.sxtw.index.nxv4i32(<vscale x 4 x i1> %1, ptr %data, <vscale x 4 x i32> %9, i32 0)
  br label %cleanup

cleanup:                                          ; preds = %for.body, %if.end
  %cmp = icmp slt i32 %add, %loopTime
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !llvm.loop !12
}

define dso_local void @test_stride_constOffset(i32 noundef %loopTime, ptr noundef %data, ptr nocapture noundef %result) local_unnamed_addr #0 {
; CHECK-LABEL: test_stride_constOffset:
; CHECK:       .LBB3_2:                                // %for.body
; CHECK:       add	x[[NEWBASE1:[0-9]+]], x1, #8
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG:[0-9]+]]/z, [x1, z{{[0-9]+}}.s, sxtw #2]
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG]]/z, [x[[NEWBASE1]], z{{[0-9]+}}.s, sxtw #2]
; CHECK:       add	x[[NEWBASE2:[0-9]+]], x1, #16
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG]]/z, [x[[NEWBASE2]], z{{[0-9]+}}.s, sxtw #2]
entry:
  %cmp9 = icmp sgt i32 %loopTime, 0
  br i1 %cmp9, label %for.body.lr.ph, label %for.cond.cleanup

for.body.lr.ph:                                   ; preds = %entry
  %.tr = tail call i32 @llvm.vscale.i32()
  %0 = shl nuw nsw i32 %.tr, 2
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body, %entry
  ret void

for.body:                                         ; preds = %for.body.lr.ph, %for.body
  %jp.010 = phi i32 [ 0, %for.body.lr.ph ], [ %conv1, %for.body ]
  %1 = tail call <vscale x 4 x i1> @llvm.aarch64.sve.whilelt.nxv4i1.i32(i32 %jp.010, i32 %loopTime)
  %2 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.index.nxv4i32(i32 %jp.010, i32 1)
  %3 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %2, <vscale x 4 x i32> zeroinitializer
  %4 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.mul.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %3, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 2, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %5 = select <vscale x 4 x i1> %1, <vscale x 4 x i32> %4, <vscale x 4 x i32> zeroinitializer
  %6 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %5, <vscale x 4 x i32> zeroinitializer)
  %7 = tail call <vscale x 4 x float> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4f32(<vscale x 4 x i1> %1, ptr %data, <vscale x 4 x i32> %6)
  %8 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %5, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 2, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %9 = tail call <vscale x 4 x float> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4f32(<vscale x 4 x i1> %1, ptr %data, <vscale x 4 x i32> %8)
  %10 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %1, <vscale x 4 x i32> %5, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 4, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %11 = tail call <vscale x 4 x float> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4f32(<vscale x 4 x i1> %1, ptr %data, <vscale x 4 x i32> %10)
  %12 = select <vscale x 4 x i1> %1, <vscale x 4 x float> %7, <vscale x 4 x float> zeroinitializer
  %13 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fadd.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %12, <vscale x 4 x float> %9)
  %14 = select <vscale x 4 x i1> %1, <vscale x 4 x float> %13, <vscale x 4 x float> zeroinitializer
  %15 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fadd.nxv4f32(<vscale x 4 x i1> %1, <vscale x 4 x float> %14, <vscale x 4 x float> %11)
  %idx.ext = sext i32 %jp.010 to i64
  %add.ptr = getelementptr inbounds float, ptr %result, i64 %idx.ext
  tail call void @llvm.masked.store.nxv4f32.p0(<vscale x 4 x float> %15, ptr %add.ptr, i32 1, <vscale x 4 x i1> %1), !tbaa !5
  %conv1 = add i32 %0, %jp.010
  %cmp = icmp slt i32 %conv1, %loopTime
  br i1 %cmp, label %for.body, label %for.cond.cleanup, !llvm.loop !13
}

define dso_local void @test_invariantOffset32bit(i32 noundef %N, i32 noundef %M, ptr noundef %matrix, ptr nocapture noundef %result) local_unnamed_addr #0 {
; CHECK-LABEL: test_invariantOffset32bit:
; CHECK:       .LBB4_5:                                // %for.body4
; CHECK:       add	x[[NEWBASE3:[0-9]+]], x2, w{{[0-9]+}}, sxtw #2
; CHECK:       add	x[[NEWBASE1:[0-9]+]], x2, w{{[0-9]+}}, sxtw #2
; CHECK:       add	x[[NEWBASE2:[0-9]+]], x2, w{{[0-9]+}}, sxtw #2
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG:[0-9]+]]/z, [x[[NEWBASE1]], z{{[0-9]+}}.s, sxtw #2]
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG]]/z, [x[[NEWBASE2]], z{{[0-9]+}}.s, sxtw #2]
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG]]/z, [x[[NEWBASE3]], z{{[0-9]+}}.s, sxtw #2]
entry:
  %cmp41 = icmp sgt i32 %N, 2
  br i1 %cmp41, label %for.cond1.preheader.lr.ph, label %for.cond.cleanup

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %div51 = udiv i32 %N, 3
  %cmp239 = icmp sgt i32 %M, 0
  %0 = sext i32 %M to i64
  %wide.trip.count = zext i32 %div51 to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %indvars.iv.next, %for.cond.cleanup3 ]
  br i1 %cmp239, label %for.body4.lr.ph, label %for.cond.cleanup3

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %1 = mul nuw nsw i64 %indvars.iv, 3
  %2 = trunc i64 %1 to i32
  %3 = mul i32 %2, %M
  %.splatinsert = insertelement <vscale x 4 x i32> poison, i32 %3, i64 0
  %.splat = shufflevector <vscale x 4 x i32> %.splatinsert, <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer
  %4 = trunc i64 %1 to i32
  %5 = add i32 %4, 1
  %6 = mul i32 %5, %M
  %.splatinsert9 = insertelement <vscale x 4 x i32> poison, i32 %6, i64 0
  %.splat10 = shufflevector <vscale x 4 x i32> %.splatinsert9, <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer
  %7 = trunc i64 %1 to i32
  %8 = add i32 %7, 2
  %9 = mul i32 %8, %M
  %.splatinsert14 = insertelement <vscale x 4 x i32> poison, i32 %9, i64 0
  %.splat15 = shufflevector <vscale x 4 x i32> %.splatinsert14, <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer
  %10 = mul nsw i64 %indvars.iv, %0
  %add.ptr = getelementptr inbounds float, ptr %result, i64 %10
  %.tr = tail call i32 @llvm.vscale.i32()
  %11 = shl nuw nsw i32 %.tr, 2
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.body4, %for.cond1.preheader
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.cond1.preheader, !llvm.loop !14

for.body4:                                        ; preds = %for.body4.lr.ph, %for.body4
  %jp.040 = phi i32 [ 0, %for.body4.lr.ph ], [ %conv20, %for.body4 ]
  %12 = tail call <vscale x 4 x i1> @llvm.aarch64.sve.whilelt.nxv4i1.i32(i32 %jp.040, i32 %M)
  %13 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.index.nxv4i32(i32 %jp.040, i32 1)
  %14 = select <vscale x 4 x i1> %12, <vscale x 4 x i32> %13, <vscale x 4 x i32> zeroinitializer
  %15 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %12, <vscale x 4 x i32> %14, <vscale x 4 x i32> %.splat)
  %16 = tail call <vscale x 4 x float> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4f32(<vscale x 4 x i1> %12, ptr %matrix, <vscale x 4 x i32> %15)
  %17 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %12, <vscale x 4 x i32> %14, <vscale x 4 x i32> %.splat10)
  %18 = tail call <vscale x 4 x float> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4f32(<vscale x 4 x i1> %12, ptr %matrix, <vscale x 4 x i32> %17)
  %19 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1> %12, <vscale x 4 x i32> %14, <vscale x 4 x i32> %.splat15)
  %20 = tail call <vscale x 4 x float> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4f32(<vscale x 4 x i1> %12, ptr %matrix, <vscale x 4 x i32> %19)
  %21 = select <vscale x 4 x i1> %12, <vscale x 4 x float> %16, <vscale x 4 x float> zeroinitializer
  %22 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fadd.nxv4f32(<vscale x 4 x i1> %12, <vscale x 4 x float> %21, <vscale x 4 x float> %18)
  %23 = select <vscale x 4 x i1> %12, <vscale x 4 x float> %22, <vscale x 4 x float> zeroinitializer
  %24 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fadd.nxv4f32(<vscale x 4 x i1> %12, <vscale x 4 x float> %23, <vscale x 4 x float> %20)
  %25 = select <vscale x 4 x i1> %12, <vscale x 4 x float> %24, <vscale x 4 x float> zeroinitializer
  %26 = tail call <vscale x 4 x float> @llvm.aarch64.sve.fdiv.nxv4f32(<vscale x 4 x i1> %12, <vscale x 4 x float> %25, <vscale x 4 x float> shufflevector (<vscale x 4 x float> insertelement (<vscale x 4 x float> poison, float 3.000000e+00, i64 0), <vscale x 4 x float> poison, <vscale x 4 x i32> zeroinitializer))
  %idx.ext17 = sext i32 %jp.040 to i64
  %add.ptr18 = getelementptr inbounds float, ptr %add.ptr, i64 %idx.ext17
  tail call void @llvm.masked.store.nxv4f32.p0(<vscale x 4 x float> %26, ptr %add.ptr18, i32 1, <vscale x 4 x i1> %12), !tbaa !5
  %conv20 = add i32 %11, %jp.040
  %cmp2 = icmp slt i32 %conv20, %M
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3, !llvm.loop !15
}

define dso_local void @test_invariantOffset64bit(i64 noundef %N, i64 noundef %M, ptr noundef %matrix, ptr nocapture noundef %result) local_unnamed_addr #0 {
; CHECK-LABEL: test_invariantOffset64bit:
; CHECK:       .LBB5_6:                                // %for.body4
; CHECK:       add	x[[NEWBASE2:[0-9]+]], x2, x{{[0-9]+}}, lsl #3
; CHECK:       add	x[[NEWBASE1:[0-9]+]], x2, x{{[0-9]+}}, lsl #3
; CHECK:       add	x[[NEWBASE3:[0-9]+]], x2, x{{[0-9]+}}, lsl #3
; CHECK:       ld1d	{ z{{[0-9]+}}.d }, p[[PG:[0-9]+]]/z, [x[[NEWBASE1]], z{{[0-9]+}}.d, lsl #3]
; CHECK:       ld1d	{ z{{[0-9]+}}.d }, p[[PG]]/z, [x[[NEWBASE2]], z{{[0-9]+}}.d, lsl #3]
; CHECK:       ld1d	{ z{{[0-9]+}}.d }, p[[PG]]/z, [x[[NEWBASE3]], z{{[0-9]+}}.d, lsl #3]
entry:
  %cmp39.not = icmp ult i64 %N, 3
  br i1 %cmp39.not, label %for.cond.cleanup, label %for.cond1.preheader.lr.ph

for.cond1.preheader.lr.ph:                        ; preds = %entry
  %div = udiv i64 %N, 3
  %cmp237.not = icmp eq i64 %M, 0
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.cond1.preheader.lr.ph, %for.cond.cleanup3
  %i.040 = phi i64 [ 0, %for.cond1.preheader.lr.ph ], [ %inc, %for.cond.cleanup3 ]
  br i1 %cmp237.not, label %for.cond.cleanup3, label %for.body4.lr.ph

for.body4.lr.ph:                                  ; preds = %for.cond1.preheader
  %mul = mul nuw i64 %i.040, 3
  %mul5 = mul i64 %mul, %M
  %.splatinsert = insertelement <vscale x 2 x i64> poison, i64 %mul5, i64 0
  %.splat = shufflevector <vscale x 2 x i64> %.splatinsert, <vscale x 2 x i64> poison, <vscale x 2 x i32> zeroinitializer
  %add7 = add nuw i64 %mul, 1
  %mul8 = mul i64 %add7, %M
  %.splatinsert9 = insertelement <vscale x 2 x i64> poison, i64 %mul8, i64 0
  %.splat10 = shufflevector <vscale x 2 x i64> %.splatinsert9, <vscale x 2 x i64> poison, <vscale x 2 x i32> zeroinitializer
  %add12 = add nuw i64 %mul, 2
  %mul13 = mul i64 %add12, %M
  %.splatinsert14 = insertelement <vscale x 2 x i64> poison, i64 %mul13, i64 0
  %.splat15 = shufflevector <vscale x 2 x i64> %.splatinsert14, <vscale x 2 x i64> poison, <vscale x 2 x i32> zeroinitializer
  %mul16 = mul i64 %i.040, %M
  %add.ptr = getelementptr inbounds double, ptr %result, i64 %mul16
  %0 = tail call i64 @llvm.vscale.i64()
  %1 = shl nuw nsw i64 %0, 1
  br label %for.body4

for.cond.cleanup:                                 ; preds = %for.cond.cleanup3, %entry
  ret void

for.cond.cleanup3:                                ; preds = %for.body4, %for.cond1.preheader
  %inc = add nuw nsw i64 %i.040, 1
  %exitcond.not = icmp eq i64 %inc, %div
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.cond1.preheader, !llvm.loop !16

for.body4:                                        ; preds = %for.body4.lr.ph, %for.body4
  %jp.038 = phi i64 [ 0, %for.body4.lr.ph ], [ %add18, %for.body4 ]
  %2 = tail call <vscale x 2 x i1> @llvm.aarch64.sve.whilelo.nxv2i1.i64(i64 %jp.038, i64 %M)
  %3 = tail call <vscale x 2 x i64> @llvm.aarch64.sve.index.nxv2i64(i64 %jp.038, i64 1)
  %4 = select <vscale x 2 x i1> %2, <vscale x 2 x i64> %3, <vscale x 2 x i64> zeroinitializer
  %5 = tail call <vscale x 2 x i64> @llvm.aarch64.sve.add.nxv2i64(<vscale x 2 x i1> %2, <vscale x 2 x i64> %4, <vscale x 2 x i64> %.splat)
  %6 = tail call <vscale x 2 x double> @llvm.aarch64.sve.ld1.gather.index.nxv2f64(<vscale x 2 x i1> %2, ptr %matrix, <vscale x 2 x i64> %5)
  %7 = tail call <vscale x 2 x i64> @llvm.aarch64.sve.add.nxv2i64(<vscale x 2 x i1> %2, <vscale x 2 x i64> %4, <vscale x 2 x i64> %.splat10)
  %8 = tail call <vscale x 2 x double> @llvm.aarch64.sve.ld1.gather.index.nxv2f64(<vscale x 2 x i1> %2, ptr %matrix, <vscale x 2 x i64> %7)
  %9 = tail call <vscale x 2 x i64> @llvm.aarch64.sve.add.nxv2i64(<vscale x 2 x i1> %2, <vscale x 2 x i64> %4, <vscale x 2 x i64> %.splat15)
  %10 = tail call <vscale x 2 x double> @llvm.aarch64.sve.ld1.gather.index.nxv2f64(<vscale x 2 x i1> %2, ptr %matrix, <vscale x 2 x i64> %9)
  %11 = select <vscale x 2 x i1> %2, <vscale x 2 x double> %6, <vscale x 2 x double> zeroinitializer
  %12 = tail call <vscale x 2 x double> @llvm.aarch64.sve.fadd.nxv2f64(<vscale x 2 x i1> %2, <vscale x 2 x double> %11, <vscale x 2 x double> %8)
  %13 = select <vscale x 2 x i1> %2, <vscale x 2 x double> %12, <vscale x 2 x double> zeroinitializer
  %14 = tail call <vscale x 2 x double> @llvm.aarch64.sve.fadd.nxv2f64(<vscale x 2 x i1> %2, <vscale x 2 x double> %13, <vscale x 2 x double> %10)
  %15 = select <vscale x 2 x i1> %2, <vscale x 2 x double> %14, <vscale x 2 x double> zeroinitializer
  %16 = tail call <vscale x 2 x double> @llvm.aarch64.sve.fdiv.nxv2f64(<vscale x 2 x i1> %2, <vscale x 2 x double> %15, <vscale x 2 x double> shufflevector (<vscale x 2 x double> insertelement (<vscale x 2 x double> poison, double 3.000000e+00, i64 0), <vscale x 2 x double> poison, <vscale x 2 x i32> zeroinitializer))
  %add.ptr17 = getelementptr inbounds double, ptr %add.ptr, i64 %jp.038
  tail call void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double> %16, ptr %add.ptr17, i32 1, <vscale x 2 x i1> %2), !tbaa !17
  %add18 = add i64 %1, %jp.038
  %cmp2 = icmp ult i64 %add18, %M
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3, !llvm.loop !19
}

define dso_local void @test_svaddx_constOffset(ptr noundef %base, <vscale x 4 x i32> %index) local_unnamed_addr #0 {
; CHECK-LABEL: test_svaddx_constOffset:
; CHECK:       .LBB6_1:                                // %for.body
; CHECK:       add	x[[NEWBASE1:[0-9]+]], x0, #40
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG:[0-9]+]]/z, [x[[NEWBASE1]], z{{[0-9]+}}.s, uxtw #2]
; CHECK:       add	x[[NEWBASE2:[0-9]+]], x0, #44
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG]]/z, [x[[NEWBASE2]], z{{[0-9]+}}.s, uxtw #2]
entry:
  %0 = tail call <vscale x 4 x i1> @llvm.aarch64.sve.ptrue.nxv4i1(i32 31)
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next.1, %for.body ]
  %index.addr.05 = phi <vscale x 4 x i32> [ %index, %entry ], [ %8, %for.body ]
  %1 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.u.nxv4i32(<vscale x 4 x i1> %0, <vscale x 4 x i32> %index.addr.05, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 10, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %2 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.ld1.gather.uxtw.index.nxv4i32(<vscale x 4 x i1> %0, ptr %base, <vscale x 4 x i32> %1)
  %3 = shl nuw nsw i64 %indvars.iv, 4
  %add.ptr = getelementptr inbounds i32, ptr %base, i64 %3
  store <vscale x 4 x i32> %2, ptr %add.ptr, align 16, !tbaa !20
  %4 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.u.nxv4i32(<vscale x 4 x i1> %0, <vscale x 4 x i32> %index.addr.05, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 1, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %5 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.u.nxv4i32(<vscale x 4 x i1> %0, <vscale x 4 x i32> %4, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 10, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %6 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.ld1.gather.uxtw.index.nxv4i32(<vscale x 4 x i1> %0, ptr %base, <vscale x 4 x i32> %5)
  %indvars.iv.next = shl i64 %indvars.iv, 4
  %7 = or i64 %indvars.iv.next, 16
  %add.ptr.1 = getelementptr inbounds i32, ptr %base, i64 %7
  store <vscale x 4 x i32> %6, ptr %add.ptr.1, align 16, !tbaa !20
  %8 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.u.nxv4i32(<vscale x 4 x i1> %0, <vscale x 4 x i32> %4, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 1, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %indvars.iv.next.1 = add nuw nsw i64 %indvars.iv, 2
  %exitcond.not.1 = icmp eq i64 %indvars.iv.next.1, 100
  br i1 %exitcond.not.1, label %for.cond.cleanup, label %for.body, !llvm.loop !22
}

define dso_local void @test_loop_invariant_offset(ptr noundef %base, <vscale x 2 x i64> %index, i64 noundef %invariant_offset) local_unnamed_addr #6 {
; CHECK-LABEL: test_loop_invariant_offset:
; CHECK:       .LBB7_1:                                // %for.body
; CHECK:       add	x[[NEWBASE:[0-9]+]], x0, x1, lsl #3
; CHECK:       st1d	{ z{{[0-9]+}}.d }, p{{[0-9]+}}, [x[[NEWBASE]], z{{[0-9]+}}.d, lsl #3]
entry:
  %0 = tail call <vscale x 2 x i1> @llvm.aarch64.sve.ptrue.nxv2i1(i32 31)
  %.splatinsert = insertelement <vscale x 2 x i64> poison, i64 %invariant_offset, i64 0
  %1 = shufflevector <vscale x 2 x i64> %.splatinsert, <vscale x 2 x i64> poison, <vscale x 2 x i32> zeroinitializer
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %index.addr.05 = phi <vscale x 2 x i64> [ %index, %entry ], [ %4, %for.body ]
  %2 = tail call <vscale x 2 x i64> @llvm.aarch64.sve.add.u.nxv2i64(<vscale x 2 x i1> %0, <vscale x 2 x i64> %index.addr.05, <vscale x 2 x i64> %1)
  %.splatinsert3 = insertelement <vscale x 2 x i64> poison, i64 %indvars.iv, i64 0
  %3 = shufflevector <vscale x 2 x i64> %.splatinsert3, <vscale x 2 x i64> poison, <vscale x 2 x i32> zeroinitializer
  tail call void @llvm.aarch64.sve.st1.scatter.index.nxv2i64(<vscale x 2 x i64> %3, <vscale x 2 x i1> %0, ptr %base, <vscale x 2 x i64> %2)
  %4 = tail call <vscale x 2 x i64> @llvm.aarch64.sve.add.u.nxv2i64(<vscale x 2 x i1> %0, <vscale x 2 x i64> %index.addr.05, <vscale x 2 x i64> shufflevector (<vscale x 2 x i64> insertelement (<vscale x 2 x i64> poison, i64 1, i64 0), <vscale x 2 x i64> poison, <vscale x 2 x i32> zeroinitializer))
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !23
}

define dso_local void @test_combined_const_and_invariant_offset(ptr noundef %base, <vscale x 4 x i32> %index, i32 noundef %invariant_offset) local_unnamed_addr #0 {
; CHECK-LABEL: test_combined_const_and_invariant_offset:
; CHECK:       .LBB8_1:                                // %for.body
; CHECK-DAG:   add	x[[NEWBASE_GPR:[0-9]+]], x0, w1, sxtw #2
; CHECK-DAG:   add	x[[NEWBASE_FINAL:[0-9]+]], x[[NEWBASE_GPR]], #40
; CHECK:       ld1w	{ z{{[0-9]+}}.s }, p[[PG:[0-9]+]]/z, [x[[NEWBASE_FINAL]], z{{[0-9]+}}.s, sxtw #2]
entry:
  %0 = tail call <vscale x 4 x i1> @llvm.aarch64.sve.ptrue.nxv4i1(i32 31)
  %.splatinsert = insertelement <vscale x 4 x i32> poison, i32 %invariant_offset, i64 0
  %.splat = shufflevector <vscale x 4 x i32> %.splatinsert, <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %index.addr.06 = phi <vscale x 4 x i32> [ %index, %entry ], [ %7, %for.body ]
  %1 = select <vscale x 4 x i1> %0, <vscale x 4 x i32> %index.addr.06, <vscale x 4 x i32> zeroinitializer
  %2 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.u.nxv4i32(<vscale x 4 x i1> %0, <vscale x 4 x i32> %1, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 10, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %3 = select <vscale x 4 x i1> %0, <vscale x 4 x i32> %2, <vscale x 4 x i32> zeroinitializer
  %4 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.u.nxv4i32(<vscale x 4 x i1> %0, <vscale x 4 x i32> %3, <vscale x 4 x i32> %.splat)
  %5 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4i32(<vscale x 4 x i1> %0, ptr %base, <vscale x 4 x i32> %4)
  %6 = shl nuw nsw i64 %indvars.iv, 4
  %add.ptr = getelementptr inbounds i32, ptr %base, i64 %6
  store <vscale x 4 x i32> %5, ptr %add.ptr, align 16, !tbaa !20
  %7 = tail call <vscale x 4 x i32> @llvm.aarch64.sve.add.u.nxv4i32(<vscale x 4 x i1> %0, <vscale x 4 x i32> %index.addr.06, <vscale x 4 x i32> shufflevector (<vscale x 4 x i32> insertelement (<vscale x 4 x i32> poison, i32 1, i64 0), <vscale x 4 x i32> poison, <vscale x 4 x i32> zeroinitializer))
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !24
}

declare <vscale x 4 x i1> @llvm.aarch64.sve.whilelt.nxv4i1.i32(i32, i32) #1
declare <vscale x 4 x i32> @llvm.aarch64.sve.index.nxv4i32(i32, i32) #1
declare <vscale x 4 x i32> @llvm.aarch64.sve.mul.nxv4i32(<vscale x 4 x i1>, <vscale x 4 x i32>, <vscale x 4 x i32>) #1
declare <vscale x 4 x float> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4f32(<vscale x 4 x i1>, ptr, <vscale x 4 x i32>) #2
declare <vscale x 4 x float> @llvm.aarch64.sve.fsubr.nxv4f32(<vscale x 4 x i1>, <vscale x 4 x float>, <vscale x 4 x float>) #1
declare <vscale x 4 x float> @llvm.aarch64.sve.fmul.nxv4f32(<vscale x 4 x i1>, <vscale x 4 x float>, <vscale x 4 x float>) #1
declare <vscale x 4 x i32> @llvm.aarch64.sve.add.nxv4i32(<vscale x 4 x i1>, <vscale x 4 x i32>, <vscale x 4 x i32>) #1
declare <vscale x 4 x float> @llvm.aarch64.sve.fsub.nxv4f32(<vscale x 4 x i1>, <vscale x 4 x float>, <vscale x 4 x float>) #1
declare <vscale x 4 x float> @llvm.aarch64.sve.fmad.nxv4f32(<vscale x 4 x i1>, <vscale x 4 x float>, <vscale x 4 x float>, <vscale x 4 x float>) #1
declare <vscale x 4 x float> @llvm.aarch64.sve.fsqrt.nxv4f32(<vscale x 4 x float>, <vscale x 4 x i1>, <vscale x 4 x float>) #1
declare float @llvm.aarch64.sve.faddv.nxv4f32(<vscale x 4 x i1>, <vscale x 4 x float>) #1
declare void @llvm.aarch64.sve.st1.scatter.sxtw.index.nxv4f32(<vscale x 4 x float>, <vscale x 4 x i1>, ptr, <vscale x 4 x i32>) #3
declare <vscale x 4 x i1> @llvm.aarch64.sve.ptrue.nxv4i1(i32 immarg) #1
declare void @llvm.aarch64.sve.prfw.gather.sxtw.index.nxv4i32(<vscale x 4 x i1>, ptr nocapture, <vscale x 4 x i32>, i32 immarg) #5
declare <vscale x 4 x float> @llvm.aarch64.sve.fadd.nxv4f32(<vscale x 4 x i1>, <vscale x 4 x float>, <vscale x 4 x float>) #1
declare <vscale x 4 x float> @llvm.aarch64.sve.fdiv.nxv4f32(<vscale x 4 x i1>, <vscale x 4 x float>, <vscale x 4 x float>) #1
declare <vscale x 2 x i1> @llvm.aarch64.sve.whilelo.nxv2i1.i64(i64, i64) #1
declare <vscale x 2 x i64> @llvm.aarch64.sve.index.nxv2i64(i64, i64) #1
declare <vscale x 2 x i64> @llvm.aarch64.sve.add.nxv2i64(<vscale x 2 x i1>, <vscale x 2 x i64>, <vscale x 2 x i64>) #1
declare <vscale x 2 x double> @llvm.aarch64.sve.ld1.gather.index.nxv2f64(<vscale x 2 x i1>, ptr, <vscale x 2 x i64>) #2
declare <vscale x 2 x double> @llvm.aarch64.sve.fadd.nxv2f64(<vscale x 2 x i1>, <vscale x 2 x double>, <vscale x 2 x double>) #1
declare <vscale x 2 x double> @llvm.aarch64.sve.fdiv.nxv2f64(<vscale x 2 x i1>, <vscale x 2 x double>, <vscale x 2 x double>) #1
declare <vscale x 4 x i32> @llvm.aarch64.sve.add.u.nxv4i32(<vscale x 4 x i1>, <vscale x 4 x i32>, <vscale x 4 x i32>) #1
declare <vscale x 4 x i32> @llvm.aarch64.sve.ld1.gather.uxtw.index.nxv4i32(<vscale x 4 x i1>, ptr, <vscale x 4 x i32>) #2
declare <vscale x 2 x i1> @llvm.aarch64.sve.ptrue.nxv2i1(i32 immarg) #1
declare <vscale x 2 x i64> @llvm.aarch64.sve.add.u.nxv2i64(<vscale x 2 x i1>, <vscale x 2 x i64>, <vscale x 2 x i64>) #1
declare void @llvm.aarch64.sve.st1.scatter.index.nxv2i64(<vscale x 2 x i64>, <vscale x 2 x i1>, ptr, <vscale x 2 x i64>) #3
declare <vscale x 4 x i32> @llvm.aarch64.sve.ld1.gather.sxtw.index.nxv4i32(<vscale x 4 x i1>, ptr, <vscale x 4 x i32>) #2
declare i64 @llvm.vscale.i64() #7
declare i32 @llvm.vscale.i32() #7
declare <vscale x 4 x float> @llvm.masked.load.nxv4f32.p0(ptr nocapture, i32 immarg, <vscale x 4 x i1>, <vscale x 4 x float>) #8
declare i1 @llvm.aarch64.sve.ptest.any.nxv4i1(<vscale x 4 x i1>, <vscale x 4 x i1>) #7
declare void @llvm.masked.store.nxv4f32.p0(<vscale x 4 x float>, ptr nocapture, i32 immarg, <vscale x 4 x i1>) #9
declare void @llvm.masked.store.nxv2f64.p0(<vscale x 2 x double>, ptr nocapture, i32 immarg, <vscale x 2 x i1>) #9

attributes #0 = { mustprogress nofree nosync nounwind memory(argmem: readwrite) uwtable vscale_range(1,16) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="hip09" "target-features"="+aes,+bf16,+crc,+dotprod,+f32mm,+f64mm,+fp-armv8,+fp16fml,+fullfp16,+i8mm,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+spe,+sve,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,-fmv" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(none) }
attributes #2 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: read) }
attributes #3 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: write) }
attributes #4 = { mustprogress nofree nosync nounwind memory(argmem: readwrite, inaccessiblemem: readwrite) uwtable vscale_range(1,16) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="hip09" "target-features"="+aes,+bf16,+crc,+dotprod,+f32mm,+f64mm,+fp-armv8,+fp16fml,+fullfp16,+i8mm,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+spe,+sve,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,-fmv" }
attributes #5 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite, inaccessiblemem: readwrite) }
attributes #6 = { mustprogress nofree nosync nounwind memory(argmem: write) uwtable vscale_range(1,16) "frame-pointer"="non-leaf" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="hip09" "target-features"="+aes,+bf16,+crc,+dotprod,+f32mm,+f64mm,+fp-armv8,+fp16fml,+fullfp16,+i8mm,+lse,+neon,+ras,+rcpc,+rdm,+sha2,+sha3,+sm4,+spe,+sve,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a,-fmv" }
attributes #7 = { nocallback nofree nosync nounwind willreturn memory(none) }
attributes #8 = { nocallback nofree nosync nounwind willreturn memory(argmem: read) }
attributes #9 = { nocallback nofree nosync nounwind willreturn memory(argmem: write) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!6, !6, i64 0}
!6 = !{!"float", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
!12 = distinct !{!12, !10}
!13 = distinct !{!13, !10}
!14 = distinct !{!14, !10}
!15 = distinct !{!15, !10}
!16 = distinct !{!16, !10}
!17 = !{!18, !18, i64 0}
!18 = !{!"double", !7, i64 0}
!19 = distinct !{!19, !10}
!20 = !{!21, !21, i64 0}
!21 = !{!"int", !7, i64 0}
!22 = distinct !{!22, !10}
!23 = distinct !{!23, !10}
!24 = distinct !{!24, !10}