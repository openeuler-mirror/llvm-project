; RUN: llc -mattr=+sve < %s | FileCheck %s --check-prefixes=DEFAULT,BOTH
; RUN: llc -mattr=+sve -aarch64-expand-ordered-fadd < %s | \
; RUN:   FileCheck %s --check-prefixes=EXPAND,BOTH --implicit-check-not=fadda

target triple = "aarch64-linux-gnu"

; Fixed-length reductions add each lane to the accumulator in order.

define half @fixed_f16(half %start, <4 x half> %v) {
; DEFAULT-LABEL: fixed_f16:
; DEFAULT:       fadda h0, p0, h0, z1.h
; EXPAND-LABEL:  fixed_f16:
; EXPAND-COUNT-4: fadd h0, h0, h{{[0-9]+}}
  %sum = call half @llvm.vector.reduce.fadd.v4f16(half %start, <4 x half> %v)
  ret half %sum
}

define float @fixed_f32(float %start, <4 x float> %v) {
; DEFAULT-LABEL: fixed_f32:
; DEFAULT:       fadda s0, p0, s0, z1.s
; EXPAND-LABEL:  fixed_f32:
; EXPAND-COUNT-4: fadd s0, s0, s{{[0-9]+}}
  %sum = call nnan float @llvm.vector.reduce.fadd.v4f32(float %start, <4 x float> %v)
  ret float %sum
}

define double @fixed_f64(double %start, <2 x double> %v) {
; DEFAULT-LABEL: fixed_f64:
; DEFAULT:       fadda d0, p0, d0, z1.d
; EXPAND-LABEL:  fixed_f64:
; EXPAND:        fadd d0, d0, d1
; EXPAND:        mov [[LANE:d[0-9]+]], v1.d[1]
; EXPAND:        fadd d0, d0, [[LANE]]
  %sum = call double @llvm.vector.reduce.fadd.v2f64(double %start, <2 x double> %v)
  ret double %sum
}

; Scalable reductions use one scalar addition per loop iteration.

define half @scalable_f16(half %start, <vscale x 8 x half> %v) {
; DEFAULT-LABEL: scalable_f16:
; DEFAULT:       fadda h0, p0, h0, z1.h
; EXPAND-LABEL:  scalable_f16:
; EXPAND:       [[LOOP:.LBB[0-9_]+]]:
; EXPAND:       fadd h0, h0, h{{[0-9]+}}
; EXPAND:       b.ne [[LOOP]]
  %sum = call half @llvm.vector.reduce.fadd.nxv8f16(half %start, <vscale x 8 x half> %v)
  ret half %sum
}

define float @scalable_f32(float %start, <vscale x 4 x float> %v) {
; DEFAULT-LABEL: scalable_f32:
; DEFAULT:       fadda s0, p0, s0, z1.s
; EXPAND-LABEL:  scalable_f32:
; EXPAND:       [[LOOP:.LBB[0-9_]+]]:
; EXPAND:       fadd s0, s0, s{{[0-9]+}}
; EXPAND:       b.ne [[LOOP]]
  %sum = call float @llvm.vector.reduce.fadd.nxv4f32(float %start, <vscale x 4 x float> %v)
  ret float %sum
}

define double @scalable_f64(double %start, <vscale x 2 x double> %v) {
; DEFAULT-LABEL: scalable_f64:
; DEFAULT:       fadda d0, p0, d0, z1.d
; EXPAND-LABEL:  scalable_f64:
; EXPAND:       [[LOOP:.LBB[0-9_]+]]:
; EXPAND:       fadd d0, d0, d{{[0-9]+}}
; EXPAND:       b.ne [[LOOP]]
  %sum = call double @llvm.vector.reduce.fadd.nxv2f64(double %start, <vscale x 2 x double> %v)
  ret double %sum
}

; A vector spanning two SVE registers normally needs two consecutive FADDAs.

define double @split_f64(double %start, <vscale x 4 x double> %v) {
; DEFAULT-LABEL: split_f64:
; DEFAULT:       fadda d0, p0, d0, z1.d
; DEFAULT-NEXT:  fadda d0, p0, d0, z2.d
; EXPAND-LABEL:  split_f64:
; EXPAND:        fadd d0, d0, d{{[0-9]+}}
  %sum = call double @llvm.vector.reduce.fadd.nxv4f64(double %start, <vscale x 4 x double> %v)
  ret double %sum
}

; Instruction selection can fold a select with -0.0 into the FADDA predicate.

define float @masked_f32(float %start, <vscale x 4 x float> %v, <vscale x 4 x i1> %mask) {
; DEFAULT-LABEL: masked_f32:
; DEFAULT:       fadda s0, p0, s0, z1.s
; EXPAND-LABEL:  masked_f32:
; EXPAND:        sel z{{[0-9]+}}.s, p0, z1.s,
; EXPAND:        fadd s0, s0, s{{[0-9]+}}
  %selected = select <vscale x 4 x i1> %mask, <vscale x 4 x float> %v, <vscale x 4 x float> splat (float -0.0)
  %sum = call float @llvm.vector.reduce.fadd.nxv4f32(float %start, <vscale x 4 x float> %selected)
  ret float %sum
}

; VP lowering combines the explicit vector length with the lane mask.

define float @vp_f32(float %start, <vscale x 4 x float> %v, <vscale x 4 x i1> %mask, i32 %evl) {
; DEFAULT-LABEL: vp_f32:
; DEFAULT:       whilelo [[EVL:p[0-9]+]].s, wzr, w0
; DEFAULT:       and [[MASK:p[0-9]+]].b, [[EVL]]/z, [[EVL]].b, p0.b
; DEFAULT:       fadda s0, [[MASK]], s0, z1.s
; EXPAND-LABEL:  vp_f32:
; EXPAND:        whilelo [[EVL:p[0-9]+]].s, wzr, w0
; EXPAND:        and [[MASK:p[0-9]+]].b, [[EVL]]/z, [[EVL]].b, p0.b
; EXPAND:        sel z{{[0-9]+}}.s, [[MASK]], z1.s,
; EXPAND:        fadd s0, s0, s{{[0-9]+}}
  %sum = call float @llvm.vp.reduce.fadd.nxv4f32(float %start, <vscale x 4 x float> %v, <vscale x 4 x i1> %mask, i32 %evl)
  ret float %sum
}

; Reassociation permits a parallel reduction in both modes.
define float @reassoc_f32(float %start, <vscale x 4 x float> %v) {
; BOTH-LABEL: reassoc_f32:
; BOTH:       faddv s{{[0-9]+}}, p0, z1.s
  %sum = call reassoc float @llvm.vector.reduce.fadd.nxv4f32(float %start, <vscale x 4 x float> %v)
  ret float %sum
}

declare half @llvm.vector.reduce.fadd.v4f16(half, <4 x half>)
declare half @llvm.vector.reduce.fadd.nxv8f16(half, <vscale x 8 x half>)
declare float @llvm.vector.reduce.fadd.v4f32(float, <4 x float>)
declare float @llvm.vector.reduce.fadd.nxv4f32(float, <vscale x 4 x float>)
declare double @llvm.vector.reduce.fadd.v2f64(double, <2 x double>)
declare double @llvm.vector.reduce.fadd.nxv2f64(double, <vscale x 2 x double>)
declare double @llvm.vector.reduce.fadd.nxv4f64(double, <vscale x 4 x double>)
declare float @llvm.vp.reduce.fadd.nxv4f32(float, <vscale x 4 x float>, <vscale x 4 x i1>, i32)
