; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt -passes=instcombine -S < %s | FileCheck %s

define float @matching_scalar(ptr dereferenceable(16) %p) {
; CHECK-LABEL: @matching_scalar(
; CHECK-NEXT:    [[R:%.*]] = load float, ptr [[P:%.*]], align 16
; CHECK-NEXT:    ret float [[R]]
;
  %r = load float, ptr %p, align 16
  ret float %r
}

define i32 @nonmatching_scalar(ptr dereferenceable(16) %p) {
; CHECK-LABEL: @nonmatching_scalar(
; CHECK-NEXT:    [[R:%.*]] = load i32, ptr [[P:%.*]], align 16
; CHECK-NEXT:    ret i32 [[R]]
;
  %r = load i32, ptr %p, align 16
  ret i32 %r
}

define i64 @larger_scalar(ptr dereferenceable(16) %p) {
; CHECK-LABEL: @larger_scalar(
; CHECK-NEXT:    [[R:%.*]] = load i64, ptr [[P:%.*]], align 16
; CHECK-NEXT:    ret i64 [[R]]
;
  %r = load i64, ptr %p, align 16
  ret i64 %r
}

define i8 @smaller_scalar(ptr dereferenceable(16) %p) {
; CHECK-LABEL: @smaller_scalar(
; CHECK-NEXT:    [[R:%.*]] = load i8, ptr [[P:%.*]], align 16
; CHECK-NEXT:    ret i8 [[R]]
;
  %r = load i8, ptr %p, align 16
  ret i8 %r
}

define i8 @smaller_scalar_less_aligned(ptr dereferenceable(16) %p) {
; CHECK-LABEL: @smaller_scalar_less_aligned(
; CHECK-NEXT:    [[R:%.*]] = load i8, ptr [[P:%.*]], align 4
; CHECK-NEXT:    ret i8 [[R]]
;
  %r = load i8, ptr %p, align 4
  ret i8 %r
}

define float @matching_scalar_small_deref(ptr dereferenceable(15) %p) {
; CHECK-LABEL: @matching_scalar_small_deref(
; CHECK-NEXT:    [[R:%.*]] = load float, ptr [[P:%.*]], align 16
; CHECK-NEXT:    ret float [[R]]
;
  %r = load float, ptr %p, align 16
  ret float %r
}

define float @matching_scalar_smallest_deref(ptr dereferenceable(1) %p) {
; CHECK-LABEL: @matching_scalar_smallest_deref(
; CHECK-NEXT:    [[R:%.*]] = load float, ptr [[P:%.*]], align 16
; CHECK-NEXT:    ret float [[R]]
;
  %r = load float, ptr %p, align 16
  ret float %r
}

define float @matching_scalar_smallest_deref_or_null(ptr dereferenceable_or_null(1) %p) {
; CHECK-LABEL: @matching_scalar_smallest_deref_or_null(
; CHECK-NEXT:    [[R:%.*]] = load float, ptr [[P:%.*]], align 16
; CHECK-NEXT:    ret float [[R]]
;
  %r = load float, ptr %p, align 16
  ret float %r
}

define float @matching_scalar_smallest_deref_addrspace(ptr addrspace(4) dereferenceable(1) %p) {
; CHECK-LABEL: @matching_scalar_smallest_deref_addrspace(
; CHECK-NEXT:    [[R:%.*]] = load float, ptr addrspace(4) [[P:%.*]], align 16
; CHECK-NEXT:    ret float [[R]]
;
  %r = load float, ptr addrspace(4) %p, align 16
  ret float %r
}

; A null pointer can't be assumed inbounds in a non-default address space.

define float @matching_scalar_smallest_deref_or_null_addrspace(ptr addrspace(4) dereferenceable_or_null(1) %p) {
; CHECK-LABEL: @matching_scalar_smallest_deref_or_null_addrspace(
; CHECK-NEXT:    [[R:%.*]] = load float, ptr addrspace(4) [[P:%.*]], align 16
; CHECK-NEXT:    ret float [[R]]
;
  %r = load float, ptr addrspace(4) %p, align 16
  ret float %r
}

define float @matching_scalar_volatile(ptr dereferenceable(16) %p) {
; CHECK-LABEL: @matching_scalar_volatile(
; CHECK-NEXT:    [[R:%.*]] = load volatile float, ptr [[P:%.*]], align 16
; CHECK-NEXT:    ret float [[R]]
;
  %r = load volatile float, ptr %p, align 16
  ret float %r
}

define float @nonvector(ptr dereferenceable(16) %p) {
; CHECK-LABEL: @nonvector(
; CHECK-NEXT:    [[R:%.*]] = load float, ptr [[P:%.*]], align 16
; CHECK-NEXT:    ret float [[R]]
;
  %r = load float, ptr %p, align 16
  ret float %r
}
