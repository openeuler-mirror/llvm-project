; RUN: opt -passes=func-merging -func-merging-threshold=0 -S < %s | FileCheck %s

; Function Attrs: noinline nounwind optnone ssp uwtable
define i32 @IntType(ptr noundef %0) #0 {
; CHECK-LABEL: IntType
; CHECK: tail call i32 @_m_f_0
; CHECK: ret
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = load i32, ptr %3, align 4
  %5 = add nsw i32 %4, 1
  store i32 %5, ptr %3, align 4
  %6 = load ptr, ptr %2, align 8
  %7 = load i32, ptr %6, align 4
  %8 = add nsw i32 %7, 2
  store i32 %8, ptr %6, align 4
  %9 = load ptr, ptr %2, align 8
  %10 = load i32, ptr %9, align 4
  %11 = add nsw i32 %10, 3
  store i32 %11, ptr %9, align 4
  %12 = load ptr, ptr %2, align 8
  %13 = load i32, ptr %12, align 4
  ret i32 %13
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @VoidType(ptr noundef %0) #0 {
; CHECK-LABEL: VoidType
; CHECK: tail call i32 @_m_f_0
; CHECK: ret
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8
  %3 = load ptr, ptr %2, align 8
  %4 = load i32, ptr %3, align 4
  %5 = add nsw i32 %4, 1
  store i32 %5, ptr %3, align 4
  %6 = load ptr, ptr %2, align 8
  %7 = load i32, ptr %6, align 4
  %8 = add nsw i32 %7, 2
  store i32 %8, ptr %6, align 4
  %9 = load ptr, ptr %2, align 8
  %10 = load i32, ptr %9, align 4
  %11 = add nsw i32 %10, 3
  store i32 %11, ptr %9, align 4
  ret void
}

; CHECK-LABEL: define internal i32 @_m_f_0
; CHECK-NEXT: entry:
; CHECK: split.bb
; CHECK-COUNT-2: load
; CHECK: br label %m.term.bb

attributes #0 = { noinline nounwind optnone ssp uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cmov,+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "tune-cpu"="generic" }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
!4 = !{!"Homebrew clang version 17.0.6"}
