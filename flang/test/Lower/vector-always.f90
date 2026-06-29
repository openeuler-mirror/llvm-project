! RUN: %flang_fc1 -emit-hlfir -o - %s | FileCheck %s

! CHECK: #loop_vectorize = #llvm.loop_vectorize<disable = false>
! CHECK: #loop_annotation = #llvm.loop_annotation<vectorize = #loop_vectorize>
! CHECK: #loop_vectorize1 = #llvm.loop_vectorize<disable = false, version = 1 : i32>
! CHECK: #loop_annotation1 = #llvm.loop_annotation<vectorize = #loop_vectorize1>
! CHECK: #loop_vectorize2 = #llvm.loop_vectorize<disable = false, version = 0 : i32>
! CHECK: #loop_annotation2 = #llvm.loop_annotation<vectorize = #loop_vectorize2>

! CHECK-LABEL: vector_always
subroutine vector_always
  integer :: a(10)
  !dir$ vector always
  !CHECK: fir.do_loop {{.*}} attributes {loopAnnotation = #loop_annotation}
  do i=1,10
     a(i)=i
  end do
end subroutine vector_always

! CHECK-LABEL: vector_version_sve
subroutine vector_version_sve
  integer :: a(10)
  !dir$ vector sve
  !CHECK: fir.do_loop {{.*}} attributes {loopAnnotation = #loop_annotation1}
  do i=1,10
     a(i)=i
  end do
end subroutine vector_version_sve

! CHECK-LABEL: vector_version_neon
subroutine vector_version_neon
  integer :: a(10)
  !dir$ vector neon
  !CHECK: fir.do_loop {{.*}} attributes {loopAnnotation = #loop_annotation2}
  do i=1,10
     a(i)=i
  end do
end subroutine vector_version_neon

! CHECK-LABEL: vector_always_version
subroutine vector_always_version
  integer :: a(10)
  !dir$ vector always
  !dir$ vector sve
  !CHECK: fir.do_loop {{.*}} attributes {loopAnnotation = #loop_annotation1}
  do i=1,10
     a(i)=i
  end do
end subroutine vector_always_version


! CHECK-LABEL: intermediate_directive
subroutine intermediate_directive
  integer :: a(10)
  !dir$ vector always
  !dir$ unknown
  !CHECK: fir.do_loop {{.*}} attributes {loopAnnotation = #loop_annotation}
  do i=1,10
     a(i)=i
  end do
end subroutine intermediate_directive
