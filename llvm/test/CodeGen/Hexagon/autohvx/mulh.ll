; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -march=hexagon -mattr=+hvxv60,+hvx-length128b,-packets < %s | FileCheck --check-prefix=V60 %s
; RUN: llc -march=hexagon -mattr=+hvxv65,+hvx-length128b,-packets < %s | FileCheck --check-prefix=V65 %s

define <32 x i32> @mulhs(<32 x i32> %a0, <32 x i32> %a1) #0 {
; V60-LABEL: mulhs:
; V60:       // %bb.0:
; V60-NEXT:    {
; V60-NEXT:     r0 = #16
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v2.w = vmpye(v1.w,v0.uh)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v0.w = vasr(v0.w,r0)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v3.w = vasr(v1.w,r0)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v5:4.w = vmpy(v0.h,v1.uh)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v31:30.w = vmpy(v0.h,v3.h)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v7:6.w = vadd(v2.uh,v4.uh)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v29:28.w = vadd(v2.h,v4.h)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v29.w += vasr(v6.w,r0)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v0.w = vadd(v29.w,v30.w)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     jumpr r31
; V60-NEXT:    }
;
; V65-LABEL: mulhs:
; V65:       // %bb.0:
; V65-NEXT:    {
; V65-NEXT:     v3:2 = vmpye(v0.w,v1.uh)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v3:2 += vmpyo(v0.w,v1.h)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v0 = v3
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     jumpr r31
; V65-NEXT:    }
  %v0 = sext <32 x i32> %a0 to <32 x i64>
  %v1 = sext <32 x i32> %a1 to <32 x i64>
  %v2 = mul <32 x i64> %v0, %v1
  %v3 = lshr <32 x i64> %v2, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %v4 = trunc <32 x i64> %v3 to <32 x i32>
  ret <32 x i32> %v4
}

define <32 x i32> @mulhu(<32 x i32> %a0, <32 x i32> %a1) #0 {
; V60-LABEL: mulhu:
; V60:       // %bb.0:
; V60-NEXT:    {
; V60-NEXT:     r0 = ##33686018
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v3:2.uw = vmpy(v0.uh,v1.uh)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     r2 = #16
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v4 = vsplat(r0)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v2.uw = vlsr(v2.uw,r2)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v1 = vdelta(v1,v4)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v1:0.uw = vmpy(v0.uh,v1.uh)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v1:0.w = vadd(v0.uh,v1.uh)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v0.w = vadd(v2.w,v0.w)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v1.w = vadd(v3.w,v1.w)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v0.uw = vlsr(v0.uw,r2)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     v0.w = vadd(v0.w,v1.w)
; V60-NEXT:    }
; V60-NEXT:    {
; V60-NEXT:     jumpr r31
; V60-NEXT:    }
;
; V65-LABEL: mulhu:
; V65:       // %bb.0:
; V65-NEXT:    {
; V65-NEXT:     r0 = ##33686018
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v3:2.uw = vmpy(v0.uh,v1.uh)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     r2 = #16
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v4 = vsplat(r0)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v2.uw = vlsr(v2.uw,r2)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v1 = vdelta(v1,v4)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v1:0.uw = vmpy(v0.uh,v1.uh)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v1:0.w = vadd(v0.uh,v1.uh)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v0.w = vadd(v2.w,v0.w)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v1.w = vadd(v3.w,v1.w)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v0.uw = vlsr(v0.uw,r2)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     v0.w = vadd(v0.w,v1.w)
; V65-NEXT:    }
; V65-NEXT:    {
; V65-NEXT:     jumpr r31
; V65-NEXT:    }
  %v0 = zext <32 x i32> %a0 to <32 x i64>
  %v1 = zext <32 x i32> %a1 to <32 x i64>
  %v2 = mul <32 x i64> %v0, %v1
  %v3 = lshr <32 x i64> %v2, <i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32, i64 32>
  %v4 = trunc <32 x i64> %v3 to <32 x i32>
  ret <32 x i32> %v4
}

attributes #0 = { nounwind }
