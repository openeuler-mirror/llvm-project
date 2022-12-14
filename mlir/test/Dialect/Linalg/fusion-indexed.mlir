// RUN: mlir-opt %s -test-linalg-greedy-fusion -split-input-file | FileCheck %s

#id_2d = affine_map<(d0, d1) -> (d0, d1)>
#pointwise_2d_trait = {
  indexing_maps = [#id_2d, #id_2d, #id_2d],
  iterator_types = ["parallel", "parallel"]
}
func.func @fuse_indexed_consumer(%A: memref<?x?xf32>,
                                    %B: memref<?x?xf32>,
                                    %C: memref<?x?xf32>,
                                    %D: memref<?x?xf32>) {
  linalg.generic #pointwise_2d_trait
    ins(%A, %B: memref<?x?xf32>, memref<?x?xf32>)
   outs(%C : memref<?x?xf32>) {
  ^bb0(%e: f32, %arg5: f32, %arg6: f32):   
    %2 = arith.addf %e, %arg5 : f32
    linalg.yield %2 : f32
  }
  %c1 = arith.constant 1 : index
  %c0 = arith.constant 0 : index
  %c25 = arith.constant 25 : index
  %c10 = arith.constant 10 : index
  %0 = memref.dim %C, %c0 : memref<?x?xf32>
  %1 = memref.dim %C, %c1 : memref<?x?xf32>
  %2 = memref.dim %D, %c0 : memref<?x?xf32>
  %3 = memref.dim %D, %c1 : memref<?x?xf32>
  scf.for %arg2 = %c0 to %0 step %c10 {
    scf.for %arg3 = %c0 to %1 step %c25 {
      %4 = memref.subview %C[%arg2, %arg3][%c10, %c25][%c1, %c1] :
          memref<?x?xf32> to memref<?x?xf32, strided<[?, ?], offset: ?>>
      %5 = memref.subview %D[%arg2, %arg3][%c10, %c25][%c1, %c1] :
          memref<?x?xf32> to memref<?x?xf32, strided<[?, ?], offset: ?>>
      linalg.generic {
        indexing_maps = [#id_2d, #id_2d],
        iterator_types = ["parallel", "parallel"]}
        ins(%4 : memref<?x?xf32, strided<[?, ?], offset: ?>>)
       outs(%5 : memref<?x?xf32, strided<[?, ?], offset: ?>>) {
      ^bb0(%arg4: f32, %arg5: f32):
        %idx0 = linalg.index 0 : index
        %idx1 = linalg.index 1 : index
        %6 = arith.addi %idx0, %arg2 : index
        %7 = arith.addi %idx1, %arg3 : index
        %8 = arith.index_cast %6 : index to i32
        %9 = arith.sitofp %8 : i32 to f32
        %10 = arith.index_cast %7 : index to i32
        %11 = arith.sitofp %10 : i32 to f32
        %12 = arith.addf %9, %11 : f32
        %13 = arith.addf %12, %arg4 : f32
        linalg.yield %13 : f32
      }
    }
  }
  return
}
// CHECK-LABEL: func @fuse_indexed_consumer
// CHECK:  scf.for
// CHECK:    scf.for
// CHECK-NOT:  scf.for
// CHECK:      linalg.generic
// CHECK-NOT:    affine.apply
// CHECK:        arith.addf
// CHECK:      linalg.generic
// CHECK:        arith.index_cast

// -----

func.func @fuse_indexed_producer(%A: memref<?x?xindex>,
                            %B: memref<?x?xindex>) {
  %c1 = arith.constant 1 : index
  %c0 = arith.constant 0 : index
  %c25 = arith.constant 25 : index
  %c10 = arith.constant 10 : index
  linalg.generic {
    indexing_maps = [affine_map<(i, j) -> (j, i)>],
    iterator_types = ["parallel", "parallel"]}
    outs(%A : memref<?x?xindex>) {
  ^bb0(%a: index):   
    %idx0 = linalg.index 0 : index
    %idx1 = linalg.index 1 : index
    %0 = arith.addi %idx0, %idx1 : index
    linalg.yield %0 : index
  }
  %A_X = memref.dim %A, %c0 : memref<?x?xindex>
  %A_Y = memref.dim %A, %c1 : memref<?x?xindex>
  scf.parallel (%arg2, %arg3) = (%c0, %c0) to (%A_X, %A_Y) step (%c10, %c25) {
    %A_view = memref.subview %A[%arg2, %arg3][%c10, %c25][%c1, %c1] :
        memref<?x?xindex> to memref<?x?xindex, strided<[?, ?], offset: ?>>
    %B_view = memref.subview %B[%arg2, %arg3][%c10, %c25][%c1, %c1] :
        memref<?x?xindex> to memref<?x?xindex, strided<[?, ?], offset: ?>>
    linalg.generic {
      indexing_maps = [affine_map<(i, j) -> (i, j)>,
                       affine_map<(i, j) -> (i, j)>],
      iterator_types = ["parallel", "parallel"]}
      ins(%A_view : memref<?x?xindex, strided<[?, ?], offset: ?>>)
      outs(%B_view : memref<?x?xindex, strided<[?, ?], offset: ?>>) {
    ^bb0(%a: index, %b: index):
      linalg.yield %a : index
    }
  }
  return
}
// CHECK: [[$MAP:#[a-zA-Z0-9_]*]] = affine_map<(d0, d1) -> (d0 + d1)>
// CHECK-LABEL: func @fuse_indexed_producer
// CHECK:  scf.parallel ([[I:%.*]], [[J:%.*]]) =
// CHECK:    linalg.generic
// CHECK:      [[idx0:%.*]] = linalg.index 0 : index
// CHECK:      [[i_new:%.*]] = affine.apply [[$MAP]]([[idx0]], [[J]])
// CHECK:      [[idx1:%.*]] = linalg.index 1 : index
// CHECK:      [[j_new:%.*]] = affine.apply [[$MAP]]([[idx1]], [[I]])
// CHECK:      [[sum:%.*]] = arith.addi [[i_new]], [[j_new]] : index
// CHECK:      linalg.yield [[sum]] : index
// CHECK:    linalg.generic

// -----

func.func @fuse_indexed_producer_tiled_second_dim_only(%A: memref<?x?xindex>,
                                                  %B: memref<?x?xindex>) {
  %c1 = arith.constant 1 : index
  %c0 = arith.constant 0 : index
  %c25 = arith.constant 25 : index
  linalg.generic {
    indexing_maps = [affine_map<(i, j) -> (i, j)>],
    iterator_types = ["parallel", "parallel"]}
    outs(%A : memref<?x?xindex>) {
  ^bb0(%a: index):   
    %idx0 = linalg.index 0 : index
    %idx1 = linalg.index 1 : index
    %0 = arith.addi %idx0, %idx1 : index
    linalg.yield %0 : index
  }
  %A_X = memref.dim %A, %c0 : memref<?x?xindex>
  %A_Y = memref.dim %A, %c1 : memref<?x?xindex>
  scf.parallel (%arg3) = (%c0) to (%A_Y) step (%c25) {
    %A_view = memref.subview %A[%c0, %arg3][%A_X, %c25][%c1, %c1] :
        memref<?x?xindex> to memref<?x?xindex, strided<[?, ?], offset: ?>>
    %B_view = memref.subview %B[%c0, %arg3][%A_X, %c25][%c1, %c1] :
        memref<?x?xindex> to memref<?x?xindex, strided<[?, ?], offset: ?>>
    linalg.generic {
      indexing_maps = [affine_map<(i, j) -> (i, j)>,
                       affine_map<(i, j) -> (i, j)>],
      iterator_types = ["parallel", "parallel"]}
      ins(%A_view : memref<?x?xindex, strided<[?, ?], offset: ?>>)
      outs(%B_view : memref<?x?xindex, strided<[?, ?], offset: ?>>) {
    ^bb0(%a: index, %b: index):
      linalg.yield %a : index
    }
  }
  return
}
// CHECK: [[$MAP:#[a-zA-Z0-9_]*]] = affine_map<(d0, d1) -> (d0 + d1)>
// CHECK-LABEL: func @fuse_indexed_producer_tiled_second_dim_only
// CHECK:  scf.parallel ([[J:%.*]]) =
// CHECK:    linalg.generic
// CHECK:      [[idx0:%.*]] = linalg.index 0 : index
// CHECK:      [[idx1:%.*]] = linalg.index 1 : index
// CHECK:      [[j_new:%.*]] = affine.apply [[$MAP]]([[idx1]], [[J]])
// CHECK:      [[sum:%.*]] = arith.addi [[idx0]], [[j_new]] : index
// CHECK:      linalg.yield [[sum]] : index
// CHECK:    linalg.generic

