; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -instcombine -S | FileCheck %s

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: readonly uwtable
define i1 @dot_ref_s(ptr noalias nocapture readonly dereferenceable(8)) {
; CHECK-LABEL: @dot_ref_s(
; CHECK-NEXT:  entry-block:
; CHECK-NEXT:    ret i1 false
;
entry-block:
  %loadedptr = load ptr, ptr %0, align 8, !nonnull !0
  %ptrtoint = ptrtoint ptr %loadedptr to i64
  %inttoptr = inttoptr i64 %ptrtoint to ptr
  %switchtmp = icmp eq ptr %inttoptr, null
  ret i1 %switchtmp

}

; Function Attrs: readonly uwtable
define ptr @function(ptr noalias nocapture readonly dereferenceable(8)) {
; CHECK-LABEL: @function(
; CHECK-NEXT:  entry-block:
; CHECK-NEXT:    [[LOADED:%.*]] = load i64, ptr [[TMP0:%.*]], align 8, !range [[RNG0:![0-9]+]]
; CHECK-NEXT:    [[INTTOPTR:%.*]] = inttoptr i64 [[LOADED]] to ptr
; CHECK-NEXT:    ret ptr [[INTTOPTR]]
;
entry-block:
  %loaded = load i64, ptr %0, align 8, !range !1
  %inttoptr = inttoptr i64 %loaded to ptr
  ret ptr %inttoptr
}


!0 = !{}
!1 = !{i64 1, i64 140737488355327}
