// RUN: mlir-opt %s --transform-interpreter --split-input-file | FileCheck %s

// CHECK-LABEL: func @set_vscale_explicit
// CHECK: %[[VAL:.*]] = arith.constant 8
// CHECK: return %[[VAL:.*]]
func.func @set_vscale_explicit() -> index {
  %0 = vector.vscale
  %1 = arith.addi %0, %0 : index
  return %1 : index
}

module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(%arg0: !transform.any_op) {
    %0 = transform.structured.match ops{["func.func"]} in %arg0 : (!transform.any_op) -> !transform.any_op
    transform.apply_patterns to %0 {
      transform.apply_patterns.set_vscale {vscale = 4 : i64}
    } : !transform.any_op
    transform.yield
  }
}

// -----

// CHECK-LABEL: func @set_vscale_default
// CHECK: %[[VAL:.*]] = arith.constant 1
// CHECK: return %[[VAL:.*]]
func.func @set_vscale_default() -> index {
  %0 = vector.vscale
  return %0 : index
}

module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(%arg0: !transform.any_op) {
    %0 = transform.structured.match ops{["func.func"]} in %arg0 : (!transform.any_op) -> !transform.any_op
    transform.apply_patterns to %0 {
      transform.apply_patterns.set_vscale
    } : !transform.any_op
    transform.yield
  }
}
