// RUN: mlir-opt -verify-diagnostics -ownership-based-buffer-deallocation -split-input-file %s

// Test Case: ownership-based-buffer-deallocation should not fail
//            with cf.assert op

// CHECK-LABEL: func @func_with_assert(
//       CHECK-SAME: %[[ARG0:.*]]: index,
//       CHECK-SAME: %[[ARG1:.*]]: index
//       CHECK: %[[CMPI:.*]] = arith.cmpi slt, %[[ARG0]], %[[ARG1]]
//       CHECK: cf.assert %[[CMPI]]
func.func @func_with_assert(%arg0: index, %arg1: index) {
  %0 = arith.cmpi slt, %arg0, %arg1 : index
  cf.assert %0, "%arg0 must be less than %arg1"
  return
}

// -----

// Test Case: ownership-based-buffer-deallocation should not fail
//            with basic blocks that contain live memrefs defined
//            in other blocks

// CHECK-LABEL: func @func_with_multi_block_memref_liveness(
//       CHECK:   %[[FIRST_ALLOC:.*]] = memref.alloc()
//       CHECK:   %[[BASE_0:[^,]+]], {{.*}} = memref.extract_strided_metadata %[[FIRST_ALLOC]]
//       CHECK:   bufferization.dealloc (%[[BASE_0]]
//       CHECK: ^bb1:
//       CHECK:   %[[SECOND_ALLOC:.*]] = memref.alloc()
//       CHECK:   %[[BASE_1:[^,]+]], {{.*}} = memref.extract_strided_metadata %[[FIRST_ALLOC]]
//       CHECK:   %[[BASE_2:[^,]+]], {{.*}} = memref.extract_strided_metadata %[[SECOND_ALLOC]]
//       CHECK:   bufferization.dealloc (%[[BASE_1]], %[[BASE_2]]
//       CHECK: ^bb2:
//       CHECK:   "test.read_buffer"(%[[FIRST_ALLOC]])
//       CHECK:   "test.read_buffer"(%[[SECOND_ALLOC]])
//       CHECK:   %[[BASE_3:[^,]+]], {{.*}} = memref.extract_strided_metadata %[[FIRST_ALLOC]]
//       CHECK:   %[[BASE_4:[^,]+]], {{.*}} = memref.extract_strided_metadata %[[SECOND_ALLOC]]
//       CHECK:   bufferization.dealloc (%[[BASE_3]], %[[BASE_4]]
module {
  func.func @func_with_multi_block_memref_liveness() {
    %alloc = memref.alloc() : memref<3x3xf32>
    cf.br ^bb1
  ^bb1:  // pred: ^bb0
    %alloc_1 = memref.alloc() : memref<4x4xf32>
    cf.br ^bb2
  ^bb2:  // 1 pred: ^bb1
    "test.read_buffer"(%alloc) : (memref<3x3xf32>) -> ()
    "test.read_buffer"(%alloc_1) : (memref<4x4xf32>) -> ()
    return
  }
}
