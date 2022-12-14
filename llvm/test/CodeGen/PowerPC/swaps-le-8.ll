; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc -verify-machineinstrs -O3 -mcpu=pwr8 \
; RUN:   -mtriple=powerpc64le-unknown-linux-gnu < %s | FileCheck %s
; RUN: llc -verify-machineinstrs -O3 -mcpu=pwr9 \
; RUN:   -mtriple=powerpc64le-unknown-linux-gnu < %s | FileCheck %s \
; RUN:   --check-prefix=CHECK-P9UP
; RUN: llc -verify-machineinstrs -O3 -mcpu=pwr10 \
; RUN:   -mtriple=powerpc64le-unknown-linux-gnu < %s | FileCheck %s \
; RUN:   --check-prefix=CHECK-P9UP
define dso_local void @test(ptr %Src, ptr nocapture %Tgt) local_unnamed_addr {
; CHECK-LABEL: test:
; CHECK:       # %bb.0: # %entry
; CHECK-NEXT:    lxvd2x 0, 0, 3
; CHECK-NEXT:    xxswapd 0, 0
; CHECK-NEXT:    stxvd2x 0, 0, 4
; CHECK-NEXT:    blr
;
; CHECK-P9UP-LABEL: test:
; CHECK-P9UP:       # %bb.0: # %entry
; CHECK-P9UP-NEXT:    lxvd2x 0, 0, 3
; CHECK-P9UP-NEXT:    stxv 0, 0(4)
; CHECK-P9UP-NEXT:    blr
entry:
  %0 = tail call <2 x double> @llvm.ppc.vsx.lxvd2x.be(ptr %Src) #2
  store <2 x double> %0, ptr %Tgt, align 1
  ret void
}

declare <2 x double> @llvm.ppc.vsx.lxvd2x.be(ptr) #1
