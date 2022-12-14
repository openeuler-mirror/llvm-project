; Test 16-bit atomic exchange.
;
; RUN: llc < %s -mtriple=s390x-linux-gnu | FileCheck %s
; RUN: llc < %s -mtriple=s390x-linux-gnu | FileCheck %s -check-prefix=CHECK-SHIFT

; Check exchange with a variable.
; - CHECK is for the main loop.
; - CHECK-SHIFT makes sure that the negated shift count used by the second
;   RLL is set up correctly.  The negation is independent of the NILL and L
;   tested in CHECK.  CHECK-SHIFT also checks that %r3 is not modified before
;   being used in the RISBG (in contrast to things like atomic addition,
;   which shift %r3 left so that %b is at the high end of the word).
define i16 @f1(ptr %src, i16 %b) {
; CHECK-LABEL: f1:
; CHECK: risbg [[RISBG:%r[1-9]+]], %r2, 0, 189, 0{{$}}
; CHECK-DAG: sll %r2, 3
; CHECK-DAG: l [[OLD:%r[0-9]+]], 0([[RISBG]])
; CHECK: [[LABEL:\.[^:]*]]:
; CHECK: rll [[ROT:%r[0-9]+]], [[OLD]], 0(%r2)
; CHECK: risbg [[ROT]], %r3, 32, 47, 16
; CHECK: rll [[NEW:%r[0-9]+]], [[ROT]], 0({{%r[1-9]+}})
; CHECK: cs [[OLD]], [[NEW]], 0([[RISBG]])
; CHECK: jl [[LABEL]]
; CHECK: rll %r2, [[OLD]], 16(%r2)
; CHECK: br %r14
;
; CHECK-SHIFT-LABEL: f1:
; CHECK-SHIFT-NOT: %r3
; CHECK-SHIFT: sll %r2, 3
; CHECK-SHIFT-NOT: %r3
; CHECK-SHIFT: lcr [[NEGSHIFT:%r[1-9]+]], %r2
; CHECK-SHIFT-NOT: %r3
; CHECK-SHIFT: rll
; CHECK-SHIFT-NOT: %r3
; CHECK-SHIFT: risbg {{%r[0-9]+}}, %r3, 32, 47, 16
; CHECK-SHIFT: rll {{%r[0-9]+}}, {{%r[0-9]+}}, 0([[NEGSHIFT]])
; CHECK-SHIFT: rll
; CHECK-SHIFT: br %r14
  %res = atomicrmw xchg ptr %src, i16 %b seq_cst
  ret i16 %res
}

; Check exchange with a constant.  We should force the constant into
; a register and use the sequence above.
define i16 @f2(ptr %src) {
; CHECK-LABEL: f2:
; CHECK: lhi [[VALUE:%r[0-9]+]], -25536
; CHECK: risbg {{%r[0-9]+}}, [[VALUE]], 32, 47, 16
; CHECK: br %r14
;
; CHECK-SHIFT-LABEL: f2:
; CHECK-SHIFT: br %r14
  %res = atomicrmw xchg ptr %src, i16 40000 seq_cst
  ret i16 %res
}
