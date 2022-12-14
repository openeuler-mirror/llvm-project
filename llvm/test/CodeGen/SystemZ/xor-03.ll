; Test 64-bit XORs in which the second operand is variable.
;
; RUN: llc < %s -mtriple=s390x-linux-gnu -mcpu=z10 | FileCheck %s
; RUN: llc < %s -mtriple=s390x-linux-gnu -mcpu=z196 | FileCheck %s

declare i64 @foo()

; Check XGR.
define i64 @f1(i64 %a, i64 %b) {
; CHECK-LABEL: f1:
; CHECK: xgr %r2, %r3
; CHECK: br %r14
  %xor = xor i64 %a, %b
  ret i64 %xor
}

; Check XG with no displacement.
define i64 @f2(i64 %a, ptr %src) {
; CHECK-LABEL: f2:
; CHECK: xg %r2, 0(%r3)
; CHECK: br %r14
  %b = load i64, ptr %src
  %xor = xor i64 %a, %b
  ret i64 %xor
}

; Check the high end of the aligned XG range.
define i64 @f3(i64 %a, ptr %src) {
; CHECK-LABEL: f3:
; CHECK: xg %r2, 524280(%r3)
; CHECK: br %r14
  %ptr = getelementptr i64, ptr %src, i64 65535
  %b = load i64, ptr %ptr
  %xor = xor i64 %a, %b
  ret i64 %xor
}

; Check the next doubleword up, which needs separate address logic.
; Other sequences besides this one would be OK.
define i64 @f4(i64 %a, ptr %src) {
; CHECK-LABEL: f4:
; CHECK: agfi %r3, 524288
; CHECK: xg %r2, 0(%r3)
; CHECK: br %r14
  %ptr = getelementptr i64, ptr %src, i64 65536
  %b = load i64, ptr %ptr
  %xor = xor i64 %a, %b
  ret i64 %xor
}

; Check the high end of the negative aligned XG range.
define i64 @f5(i64 %a, ptr %src) {
; CHECK-LABEL: f5:
; CHECK: xg %r2, -8(%r3)
; CHECK: br %r14
  %ptr = getelementptr i64, ptr %src, i64 -1
  %b = load i64, ptr %ptr
  %xor = xor i64 %a, %b
  ret i64 %xor
}

; Check the low end of the XG range.
define i64 @f6(i64 %a, ptr %src) {
; CHECK-LABEL: f6:
; CHECK: xg %r2, -524288(%r3)
; CHECK: br %r14
  %ptr = getelementptr i64, ptr %src, i64 -65536
  %b = load i64, ptr %ptr
  %xor = xor i64 %a, %b
  ret i64 %xor
}

; Check the next doubleword down, which needs separate address logic.
; Other sequences besides this one would be OK.
define i64 @f7(i64 %a, ptr %src) {
; CHECK-LABEL: f7:
; CHECK: agfi %r3, -524296
; CHECK: xg %r2, 0(%r3)
; CHECK: br %r14
  %ptr = getelementptr i64, ptr %src, i64 -65537
  %b = load i64, ptr %ptr
  %xor = xor i64 %a, %b
  ret i64 %xor
}

; Check that XG allows an index.
define i64 @f8(i64 %a, i64 %src, i64 %index) {
; CHECK-LABEL: f8:
; CHECK: xg %r2, 524280({{%r4,%r3|%r3,%r4}})
; CHECK: br %r14
  %add1 = add i64 %src, %index
  %add2 = add i64 %add1, 524280
  %ptr = inttoptr i64 %add2 to ptr
  %b = load i64, ptr %ptr
  %xor = xor i64 %a, %b
  ret i64 %xor
}

; Check that XORs of spilled values can use OG rather than OGR.
define i64 @f9(ptr %ptr0) {
; CHECK-LABEL: f9:
; CHECK: brasl %r14, foo@PLT
; CHECK: xg %r2, 160(%r15)
; CHECK: br %r14
  %ptr1 = getelementptr i64, ptr %ptr0, i64 2
  %ptr2 = getelementptr i64, ptr %ptr0, i64 4
  %ptr3 = getelementptr i64, ptr %ptr0, i64 6
  %ptr4 = getelementptr i64, ptr %ptr0, i64 8
  %ptr5 = getelementptr i64, ptr %ptr0, i64 10
  %ptr6 = getelementptr i64, ptr %ptr0, i64 12
  %ptr7 = getelementptr i64, ptr %ptr0, i64 14
  %ptr8 = getelementptr i64, ptr %ptr0, i64 16
  %ptr9 = getelementptr i64, ptr %ptr0, i64 18

  %val0 = load i64, ptr %ptr0
  %val1 = load i64, ptr %ptr1
  %val2 = load i64, ptr %ptr2
  %val3 = load i64, ptr %ptr3
  %val4 = load i64, ptr %ptr4
  %val5 = load i64, ptr %ptr5
  %val6 = load i64, ptr %ptr6
  %val7 = load i64, ptr %ptr7
  %val8 = load i64, ptr %ptr8
  %val9 = load i64, ptr %ptr9

  %ret = call i64 @foo()

  %xor0 = xor i64 %ret, %val0
  %xor1 = xor i64 %xor0, %val1
  %xor2 = xor i64 %xor1, %val2
  %xor3 = xor i64 %xor2, %val3
  %xor4 = xor i64 %xor3, %val4
  %xor5 = xor i64 %xor4, %val5
  %xor6 = xor i64 %xor5, %val6
  %xor7 = xor i64 %xor6, %val7
  %xor8 = xor i64 %xor7, %val8
  %xor9 = xor i64 %xor8, %val9

  ret i64 %xor9
}
