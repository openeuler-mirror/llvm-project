; RUN: llc -mtriple=aarch64-linux-gnu -mattr=+sve,+bf16 < %s | FileCheck %s

; Check that scalar bf16 bitcasted sign-bit logic is not folded to FABS/FNEG
; nodes that AArch64 cannot select.

define <vscale x 8 x bfloat> @bf16_abs_bitlogic(<vscale x 8 x bfloat> %a) {
; CHECK-LABEL: bf16_abs_bitlogic:
; CHECK:       // %bb.0:
; CHECK-NEXT:    and z0.h, z0.h, #0x7fff
; CHECK-NEXT:    ret
  %i = bitcast <vscale x 8 x bfloat> %a to <vscale x 8 x i16>
  %m0 = insertelement <vscale x 8 x i16> poison, i16 32767, i64 0
  %m = shufflevector <vscale x 8 x i16> %m0, <vscale x 8 x i16> poison, <vscale x 8 x i32> zeroinitializer
  %and = and <vscale x 8 x i16> %i, %m
  %r = bitcast <vscale x 8 x i16> %and to <vscale x 8 x bfloat>
  ret <vscale x 8 x bfloat> %r
}

define <vscale x 8 x bfloat> @bf16_neg_bitlogic(<vscale x 8 x bfloat> %a) {
; CHECK-LABEL: bf16_neg_bitlogic:
; CHECK:       // %bb.0:
; CHECK-NEXT:    eor z0.h, z0.h, #0x8000
; CHECK-NEXT:    ret
  %i = bitcast <vscale x 8 x bfloat> %a to <vscale x 8 x i16>
  %m0 = insertelement <vscale x 8 x i16> poison, i16 -32768, i64 0
  %m = shufflevector <vscale x 8 x i16> %m0, <vscale x 8 x i16> poison, <vscale x 8 x i32> zeroinitializer
  %xor = xor <vscale x 8 x i16> %i, %m
  %r = bitcast <vscale x 8 x i16> %xor to <vscale x 8 x bfloat>
  ret <vscale x 8 x bfloat> %r
}

define <vscale x 8 x bfloat> @bf16_nabs_bitlogic(<vscale x 8 x bfloat> %a) {
; CHECK-LABEL: bf16_nabs_bitlogic:
; CHECK:       // %bb.0:
; CHECK-NEXT:    orr z0.h, z0.h, #0x8000
; CHECK-NEXT:    ret
  %i = bitcast <vscale x 8 x bfloat> %a to <vscale x 8 x i16>
  %m0 = insertelement <vscale x 8 x i16> poison, i16 -32768, i64 0
  %m = shufflevector <vscale x 8 x i16> %m0, <vscale x 8 x i16> poison, <vscale x 8 x i32> zeroinitializer
  %or = or <vscale x 8 x i16> %i, %m
  %r = bitcast <vscale x 8 x i16> %or to <vscale x 8 x bfloat>
  ret <vscale x 8 x bfloat> %r
}