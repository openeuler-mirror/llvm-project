; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -passes=aggressive-instcombine -S | FileCheck %s

define i16 @trunc_shl(i16 %x, i16 %y) {
; CHECK-LABEL: @trunc_shl(
; CHECK-NEXT:    [[CMP0:%.*]] = icmp ult i16 [[Y:%.*]], 16
; CHECK-NEXT:    call void @llvm.assume(i1 [[CMP0]])
; CHECK-NEXT:    [[I0:%.*]] = shl i16 [[X:%.*]], [[Y]]
; CHECK-NEXT:    ret i16 [[I0]]
;
  %cmp0 = icmp ult i16 %y, 16
  call void @llvm.assume(i1 %cmp0)

  %zextx = zext i16 %x to i32
  %zexty = zext i16 %y to i32

  %i0 = shl i32 %zextx, %zexty
  %r = trunc i32 %i0 to i16
  ret i16 %r
}

define i16 @trunc_lshr(i16 %x, i16 %y) {
; CHECK-LABEL: @trunc_lshr(
; CHECK-NEXT:    [[CMP0:%.*]] = icmp ult i16 [[X:%.*]], 0
; CHECK-NEXT:    [[CMP1:%.*]] = icmp ult i16 [[Y:%.*]], 16
; CHECK-NEXT:    call void @llvm.assume(i1 [[CMP0]])
; CHECK-NEXT:    call void @llvm.assume(i1 [[CMP1]])
; CHECK-NEXT:    [[I0:%.*]] = lshr i16 [[X]], [[Y]]
; CHECK-NEXT:    ret i16 [[I0]]
;
  %cmp0 = icmp ult i16 %x, 65536
  %cmp1 = icmp ult i16 %y, 16
  call void @llvm.assume(i1 %cmp0)
  call void @llvm.assume(i1 %cmp1)

  %zextx = zext i16 %x to i32
  %zexty = zext i16 %y to i32

  %i0 = lshr i32 %zextx, %zexty
  %r = trunc i32 %i0 to i16
  ret i16 %r
}

define i16 @trunc_ashr(i16 %x, i16 %y) {
; CHECK-LABEL: @trunc_ashr(
; CHECK-NEXT:    [[CMP0:%.*]] = icmp slt i16 [[X:%.*]], 32767
; CHECK-NEXT:    [[CMP1:%.*]] = icmp sge i16 [[X]], -32768
; CHECK-NEXT:    [[CMP2:%.*]] = icmp ult i16 [[Y:%.*]], 16
; CHECK-NEXT:    call void @llvm.assume(i1 [[CMP0]])
; CHECK-NEXT:    call void @llvm.assume(i1 [[CMP1]])
; CHECK-NEXT:    call void @llvm.assume(i1 [[CMP2]])
; CHECK-NEXT:    [[I0:%.*]] = ashr i16 [[X]], [[Y]]
; CHECK-NEXT:    ret i16 [[I0]]
;
  %cmp0 = icmp slt i16 %x, 32767
  %cmp1 = icmp sge i16 %x, -32768
  %cmp2 = icmp ult i16 %y, 16
  call void @llvm.assume(i1 %cmp0)
  call void @llvm.assume(i1 %cmp1)
  call void @llvm.assume(i1 %cmp2)

  %zextx = sext i16 %x to i32
  %zexty = sext i16 %y to i32

  %i0 = ashr i32 %zextx, %zexty
  %r = trunc i32 %i0 to i16
  ret i16 %r
}

declare void @llvm.assume(i1)
