// RUN: mlir-opt -finalize-memref-to-llvm -buffer-deallocation-pipeline %s -split-input-file | FileCheck %s
//

// Check memref.assume_alignment is transomed to llvm.assume and buffer-deallocation-pipeline is considered
// as safe operation

// CHECK-LABEL: func @load_and_assume(
// CHECK-SAME: %[[ARG0:.*]]: memref<?x?xf32, strided<[?, ?], offset: ?>>,
// CHECK: %[[DESC:.*]] = builtin.unrealized_conversion_cast %[[ARG0]] : memref<?x?xf32, strided<[?, ?], offset: ?>> to !llvm.struct<(ptr, ptr, i64, array<2 x i64>, array<2 x i64>)>
// CHECK: %[[ALIGNED_PTR:.*]] = llvm.extractvalue %[[DESC]][1] : !llvm.struct<(ptr, ptr, i64, array<2 x i64>, array<2 x i64>)>
// CHECK: %[[OFFSET:.*]] = llvm.extractvalue %[[DESC]][2] : !llvm.struct<(ptr, ptr, i64, array<2 x i64>, array<2 x i64>)>
// CHECK: %[[BUFF_ADDR:.*]] = llvm.getelementptr %[[ALIGNED_PTR]][%[[OFFSET]]] : (!llvm.ptr, i64) -> !llvm.ptr, f32
// CHECK: llvm.intr.assume %{{.*}} ["align"(%[[BUFF_ADDR]], %{{.*}} : !llvm.ptr, i64)] : i1
// CHECK: %[[LD_ADDR:.*]] = llvm.getelementptr %[[BUFF_ADDR]][%{{.*}}] : (!llvm.ptr, i64) -> !llvm.ptr, f32
// CHECK: %[[VAL:.*]] = llvm.load %[[LD_ADDR]] : !llvm.ptr -> f32
// CHECK: return %[[VAL]] : f32
func.func @load_and_assume(
    %arg0: memref<?x?xf32, strided<[?, ?], offset: ?>>,
    %i0: index, %i1: index)
    -> f32 {
  memref.assume_alignment %arg0, 16 : memref<?x?xf32, strided<[?, ?], offset: ?>>
  %2 = memref.load %arg0[%i0, %i1] : memref<?x?xf32, strided<[?, ?], offset: ?>>
  func.return %2 : f32
}
