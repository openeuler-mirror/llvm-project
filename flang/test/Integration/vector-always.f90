! RUN: %flang_fc1 -emit-llvm -o - %s | FileCheck %s

! CHECK-LABEL: vector_always
subroutine vector_always
  integer :: a(10)
  !dir$ vector always
  ! CHECK:   br i1 {{.*}}, label {{.*}}, label {{.*}}, !llvm.loop ![[ANNOTATION:.*]]
  do i=1,10
     a(i)=i
  end do
end subroutine vector_always

! CHECK-LABEL: vector_version_sve
subroutine vector_version_sve
  integer :: a(10)
  !dir$ vector sve
  ! CHECK:   br i1 {{.*}}, label {{.*}}, label {{.*}}, !llvm.loop ![[SVE_ANNOTATION:.*]]
  do i=1,10
     a(i)=i
  end do
end subroutine vector_version_sve

! CHECK-LABEL: vector_version_neon
subroutine vector_version_neon
  integer :: a(10)
  !dir$ vector neon
  ! CHECK:   br i1 {{.*}}, label {{.*}}, label {{.*}}, !llvm.loop ![[NEON_ANNOTATION:.*]]
  do i=1,10
     a(i)=i
  end do
end subroutine vector_version_neon

! CHECK: ![[ANNOTATION]] = distinct !{![[ANNOTATION]], ![[VECTORIZE:.*]]}
! CHECK: ![[VECTORIZE]] = !{!"llvm.loop.vectorize.enable", i1 true}
! CHECK: ![[SVE_ANNOTATION]] = distinct !{![[SVE_ANNOTATION]], ![[SVE_VECTORIZE:.*]], ![[SVE_VERSION:.*]]}
! CHECK: ![[SVE_VECTORIZE]] = !{!"llvm.loop.vectorize.enable", i1 true}
! CHECK: ![[SVE_VERSION]] = !{!"llvm.loop.vectorize.version", i32 1}
! CHECK: ![[NEON_ANNOTATION]] = distinct !{![[NEON_ANNOTATION]], ![[NEON_VECTORIZE:.*]], ![[NEON_VERSION:.*]]}
! CHECK: ![[NEON_VECTORIZE]] = !{!"llvm.loop.vectorize.enable", i1 true}
! CHECK: ![[NEON_VERSION]] = !{!"llvm.loop.vectorize.version", i32 0}
