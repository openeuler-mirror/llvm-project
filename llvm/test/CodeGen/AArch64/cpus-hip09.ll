; REQUIRES: enable_enable_aarch64_hip09
; This tests that llc accepts all valid AArch64 CPUs

; RUN: llc < %s -mtriple=arm64-unknown-unknown -mcpu=hip09 2>&1 | FileCheck %s

; CHECK-NOT: {{.*}}  is not a recognized processor for this target
; INVALID: {{.*}}  is not a recognized processor for this target

define i32 @f(i64 %z) {
	ret i32 0
}
