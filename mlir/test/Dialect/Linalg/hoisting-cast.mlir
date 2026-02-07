// RUN: mlir-opt -transform-interpreter -canonicalize --split-input-file --allow-unregistered-dialect %s | FileCheck %s

// CHECK-LABEL: func @hoist_vector_casts(
//  CHECK-SAME:   %[[TENSOR:[a-zA-Z0-9]*]]: tensor<4x4x2xf32>,
//  CHECK-SAME:   %[[DEFAULT:[a-zA-Z0-9]*]]: f32,
//  CHECK-SAME:   %[[LB:[a-zA-Z0-9]*]]: index,
//  CHECK-SAME:   %[[UB:[a-zA-Z0-9]*]]: index,
//  CHECK-SAME:   %[[STEP:[a-zA-Z0-9]*]]: index
func.func @hoist_vector_casts(
    %tensor: tensor<4x4x2xf32>,
    %default: f32, %lb : index, %ub : index, %step: index) {
  // CHECK: arith.constant 0 : index
  // CHECK-NEXT: %[[VECTOR0:.*]] = vector.transfer_read %{{.*}} : tensor<4x4x2xf32>, vector<4x4x2xf32>
  // CHECK-NEXT: %[[VECTOR1:.*]] = vector.shape_cast %[[VECTOR0]] : vector<4x4x2xf32> to vector<4x8xf32>
  // CHECK-NEXT: %[[LOOP:.*]] = scf.for {{.*}} iter_args(%[[ARG:.*]] = %[[VECTOR1]]) -> (vector<4x8xf32>) {
  // CHECK-NEXT: %[[U:.*]] = "some_use"(%[[ARG]]) : (vector<4x8xf32>) -> vector<4x8xf32>
  // CHECK-NEXT: scf.yield %[[U]] : vector<4x8xf32>
  // CHECK-NEXT: }
  // CHECK-NEXT: %[[VECTOR2:.*]] = vector.shape_cast %[[LOOP]] : vector<4x8xf32> to vector<4x4x2xf32>
  %zero = arith.constant 0 : index
  %vector = vector.transfer_read %tensor[%zero, %zero, %zero], %default {in_bounds = [true, true, true]} : tensor<4x4x2xf32>, vector<4x4x2xf32>
  %loop = scf.for %i = %lb to %ub step %step iter_args(%arg = %vector) -> (vector<4x4x2xf32>) {
    %c = vector.shape_cast %arg : vector<4x4x2xf32> to vector<4x8xf32>
    %u = "some_use"(%c) : (vector<4x8xf32>) -> vector<4x8xf32>
    %res = vector.shape_cast %u : vector<4x8xf32> to vector<4x4x2xf32>
    scf.yield %res: vector<4x4x2xf32>
  }
  %u = "some_use"(%loop) : (vector<4x4x2xf32>) -> vector<4x4x2xf32>
  return
}

module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(%arg1: !transform.any_op {transform.readonly}) {
    %0 = transform.structured.match ops{["func.func"]} in %arg1
      : (!transform.any_op) -> !transform.any_op
    transform.structured.hoist_redundant_vector_shape_casts %0
      : (!transform.any_op) -> !transform.any_op
    transform.yield
  }
}

// -----

// CHECK-LABEL: func @hoist_vector_casts_multiargs(
//  CHECK-SAME:   %[[TENSOR:[a-zA-Z0-9]*]]: tensor<4x4x2xf32>,
//  CHECK-SAME:   %[[DEFAULT:[a-zA-Z0-9]*]]: f32,
//  CHECK-SAME:   %[[LB:[a-zA-Z0-9]*]]: index,
//  CHECK-SAME:   %[[UB:[a-zA-Z0-9]*]]: index,
//  CHECK-SAME:   %[[STEP:[a-zA-Z0-9]*]]: index
func.func @hoist_vector_casts_multiargs(
    %tensor: tensor<4x4x2xf32>,
    %default: f32, %lb : index, %ub : index, %step: index) {
  // CHECK-NEXT: arith.constant 0 : index
  // CHECK-NEXT: %[[V0:.*]] = vector.transfer_read %{{.*}} : tensor<4x4x2xf32>, vector<4x4x2xf32>
  // CHECK-NEXT: %[[V1:.*]] = vector.shape_cast %[[V0]] : vector<4x4x2xf32> to vector<4x8xf32>
  // CHECK-NEXT: %[[LOOP:.*]]:2 = scf.for {{.*}} iter_args(%[[ARG0:.*]] = %[[V0]], %[[ARG1:.*]] = %[[V1]])
  // CHECK-NEXT: %[[U0:.*]] = "some_use"(%[[ARG0]]) : (vector<4x4x2xf32>) -> vector<4x4x2xf32>
  // CHECK-NEXT: %[[U1:.*]] = "some_use"(%[[ARG1]]) : (vector<4x8xf32>) -> vector<4x8xf32>
  // CHECK-NEXT: scf.yield %[[U0]], %[[U1]] : vector<4x4x2xf32>, vector<4x8xf32>
  // CHECK-NEXT: }
  // CHECK-NEXT: %[[RES:.*]] = vector.shape_cast %[[LOOP]]#1 : vector<4x8xf32> to vector<4x4x2xf32>
  %zero = arith.constant 0 : index
  %vector = vector.transfer_read %tensor[%zero, %zero, %zero], %default {in_bounds = [true, true, true]} : tensor<4x4x2xf32>, vector<4x4x2xf32>
  %loop:2 = scf.for %i = %lb to %ub step %step iter_args(%arg0 = %vector, %arg1 = %vector) -> (vector<4x4x2xf32>, vector<4x4x2xf32>) {
    %c = vector.shape_cast %arg1 : vector<4x4x2xf32> to vector<4x8xf32>
    %u0 = "some_use"(%arg0) : (vector<4x4x2xf32>) -> vector<4x4x2xf32>
    %u1 = "some_use"(%c) : (vector<4x8xf32>) -> vector<4x8xf32>
    %res = vector.shape_cast %u1 : vector<4x8xf32> to vector<4x4x2xf32>
    scf.yield %u0, %res: vector<4x4x2xf32>, vector<4x4x2xf32>
  }
  %u0 = "some_use"(%loop#0) : (vector<4x4x2xf32>) -> vector<4x4x2xf32>
  %u1 = "some_use"(%loop#1) : (vector<4x4x2xf32>) -> vector<4x4x2xf32>
  return
}

module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(%arg1: !transform.any_op {transform.readonly}) {
    %0 = transform.structured.match ops{["func.func"]} in %arg1
      : (!transform.any_op) -> !transform.any_op
    transform.structured.hoist_redundant_vector_shape_casts %0
      : (!transform.any_op) -> !transform.any_op
    transform.yield
  }
}
