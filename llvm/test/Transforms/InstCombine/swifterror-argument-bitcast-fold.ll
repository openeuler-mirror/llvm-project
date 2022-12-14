; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt -passes=instcombine -S %s | FileCheck %s

; The swifterror value can only be loaded, stored or used as swifterror
; argument. Make sure we do not try to turn the function bitcast into an
; argument bitcast.
define swiftcc void @spam(ptr swifterror %arg) {
; CHECK-LABEL: @spam(
; CHECK-NEXT:  bb:
; CHECK-NEXT:    call swiftcc void @widget(ptr swifterror [[ARG:%.*]])
; CHECK-NEXT:    ret void
;
bb:
  call swiftcc void @widget(ptr swifterror %arg)
  ret void
}

declare swiftcc void @widget(ptr)
