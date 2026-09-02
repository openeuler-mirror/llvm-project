// RUN: mlir-opt %s -transform-interpreter -split-input-file | FileCheck %s

#map = affine_map<(d0, d1) -> (d0, d1)>

func.func @shared_empty(%arg0: tensor<4x1xi32>)
    -> (tensor<4x1xi32>, tensor<4x1xi32>) {
  %empty = tensor.empty() : tensor<4x1xi32>
  %0 = linalg.generic {
      indexing_maps = [#map, #map],
      iterator_types = ["parallel", "parallel"]}
      ins(%arg0 : tensor<4x1xi32>) outs(%empty : tensor<4x1xi32>) {
    ^bb0(%in: i32, %out: i32):
      linalg.yield %in : i32
  } -> tensor<4x1xi32>
  %1 = linalg.generic {
      indexing_maps = [#map, #map],
      iterator_types = ["parallel", "parallel"]}
      ins(%arg0 : tensor<4x1xi32>) outs(%empty : tensor<4x1xi32>) {
    ^bb0(%in: i32, %out: i32):
      linalg.yield %in : i32
  } -> tensor<4x1xi32>
  return %0, %1 : tensor<4x1xi32>, tensor<4x1xi32>
}

module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(
      %root: !transform.any_op {transform.readonly}) {
    %func = transform.structured.match ops{["func.func"]} in %root
        : (!transform.any_op) -> !transform.any_op
    transform.apply_patterns to %func {
      transform.apply_patterns.linalg.fold_unit_extent_dims_via_slices
    } : !transform.any_op
    transform.yield
  }
}

// CHECK-LABEL: func.func @shared_empty
// CHECK-NOT: tensor.extract_slice
// CHECK-COUNT-2: iterator_types = ["parallel", "parallel"]
// CHECK: return

// -----

#map = affine_map<(d0, d1) -> (d0, d1)>

func.func @read_write_alias(%arg0: tensor<4x1xi32>) -> tensor<4x1xi32> {
  %0 = linalg.generic {
      indexing_maps = [#map, #map],
      iterator_types = ["parallel", "parallel"]}
      ins(%arg0 : tensor<4x1xi32>) outs(%arg0 : tensor<4x1xi32>) {
    ^bb0(%in: i32, %out: i32):
      %1 = arith.addi %in, %out : i32
      linalg.yield %1 : i32
  } -> tensor<4x1xi32>
  return %0 : tensor<4x1xi32>
}

module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(
      %root: !transform.any_op {transform.readonly}) {
    %func = transform.structured.match ops{["func.func"]} in %root
        : (!transform.any_op) -> !transform.any_op
    transform.apply_patterns to %func {
      transform.apply_patterns.linalg.fold_unit_extent_dims_via_slices
    } : !transform.any_op
    transform.yield
  }
}

// CHECK-LABEL: func.func @read_write_alias
// CHECK-NOT: tensor.extract_slice
// CHECK: iterator_types = ["parallel", "parallel"]
// CHECK: return

// -----

#map = affine_map<(d0, d1) -> (d0, d1)>

func.func @unique_empty(%arg0: tensor<4x1xi32>) -> tensor<4x1xi32> {
  %empty = tensor.empty() : tensor<4x1xi32>
  %0 = linalg.generic {
      indexing_maps = [#map, #map],
      iterator_types = ["parallel", "parallel"]}
      ins(%arg0 : tensor<4x1xi32>) outs(%empty : tensor<4x1xi32>) {
    ^bb0(%in: i32, %out: i32):
      linalg.yield %in : i32
  } -> tensor<4x1xi32>
  return %0 : tensor<4x1xi32>
}

module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(
      %root: !transform.any_op {transform.readonly}) {
    %func = transform.structured.match ops{["func.func"]} in %root
        : (!transform.any_op) -> !transform.any_op
    transform.apply_patterns to %func {
      transform.apply_patterns.linalg.fold_unit_extent_dims_via_slices
    } : !transform.any_op
    transform.yield
  }
}

// CHECK-LABEL: func.func @unique_empty
// CHECK: tensor.extract_slice
// CHECK: iterator_types = ["parallel"]
// CHECK: return
