// RUN: mlir-opt %s --transform-interpreter --canonicalize | FileCheck %s

// CHECK-LABEL: func @outerproduct4x8
// CHECK-SAME: %[[A:.*]]: vector<4xf32>, %[[B:.*]]: vector<8xf32>, %[[C:.*]]: vector<4x8xf32>
// CHECK-DAG: %[[C0:.*]] = vector.extract  %[[C]][0] : vector<8xf32> from vector<4x8xf32
// CHECK-DAG: %[[A0:.*]] = vector.scalable.insert %[[A]]{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[A1:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.dupq.lane"(%[[A0]], %c0_i64)
// CHECK-DAG: %[[C1:.*]] = vector.scalable.insert %[[C0]],{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[B0:.*]] = vector.scalable.insert %[[B]],{{.*}} into vector<[4]xf32>
// CHECK: %[[FMLA0:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.fmla.lane"(%[[C1]], %[[B0]], %[[A1]], %c0_i32){{.*}} vector<[4]xf32>
// CHECK-NEXT: %[[R0:.*]] = vector.scalable.extract %[[FMLA0]][0] : vector<8xf32> from vector<[4]xf32>
// CHECK-NEXT: %[[I0:.*]] = vector.insert %[[R0]], %cst [0] : vector<8xf32> into vector<4x8xf32>
// CHECK-DAG: %[[A2:.*]] = vector.scalable.insert %[[A]]{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[A3:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.dupq.lane"(%[[A2]], %c0_i64)
// CHECK-DAG: %[[C2:.*]] = vector.extract %[[C]][1] : vector<8xf32> from vector<4x8xf32>
// CHECK-DAG: %[[C3:.*]] = vector.scalable.insert %[[C2]],{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[B1:.*]] = vector.scalable.insert %[[B]],{{.*}} into vector<[4]xf32>
// CHECK: %[[FMLA1:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.fmla.lane"(%[[C3]], %[[B1]], %[[A3]], %c1_i32){{.*}} vector<[4]xf32>
// CHECK-NEXT: %[[R1:.*]] = vector.scalable.extract %[[FMLA1]][0] : vector<8xf32> from vector<[4]xf32>
// CHECK-NEXT: %[[I1:.*]] = vector.insert %[[R1]], %[[I0]] [1] : vector<8xf32> into vector<4x8xf32>
// CHECK-DAG: %[[A4:.*]] = vector.scalable.insert %[[A]]{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[A5:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.dupq.lane"(%[[A4]], %c0_i64)
// CHECK-DAG: %[[C4:.*]] = vector.extract %[[C]][2] : vector<8xf32> from vector<4x8xf32>
// CHECK-DAG: %[[C5:.*]] = vector.scalable.insert %[[C4]],{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[B2:.*]] = vector.scalable.insert %[[B]],{{.*}} into vector<[4]xf32>
// CHECK: %[[FMLA2:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.fmla.lane"(%[[C5]], %[[B2]], %[[A5]], %c2_i32){{.*}} vector<[4]xf32>
// CHECK-NEXT: %[[R2:.*]] = vector.scalable.extract %[[FMLA2]][0] : vector<8xf32> from vector<[4]xf32>
// CHECK-NEXT: %[[I2:.*]] = vector.insert %[[R2]], %[[I1]] [2] : vector<8xf32> into vector<4x8xf32>
// CHECK-DAG: %[[A6:.*]] = vector.scalable.insert %[[A]]{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[A7:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.dupq.lane"(%[[A6]], %c0_i64)
// CHECK-DAG: %[[C6:.*]] = vector.extract %[[C]][3] : vector<8xf32> from vector<4x8xf32>
// CHECK-DAG: %[[C7:.*]] = vector.scalable.insert %[[C6]],{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[B3:.*]] = vector.scalable.insert %[[B]],{{.*}} into vector<[4]xf32>
// CHECK: %[[FMLA3:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.fmla.lane"(%[[C7]], %[[B3]], %[[A7]], %c3_i32){{.*}} vector<[4]xf32>
// CHECK-NEXT: %[[R3:.*]] = vector.scalable.extract %[[FMLA3]][0] : vector<8xf32> from vector<[4]xf32>
// CHECK-NEXT: %[[I3:.*]] = vector.insert %[[R3]], %[[I2]] [3] : vector<8xf32> into vector<4x8xf32>
// CHECK: return %[[I3]]
func.func @outerproduct4x8(%arg0 : vector<4xf32>, %arg1 : vector<8xf32>, %arg2 : vector<4x8xf32>) ->vector<4x8xf32> {
    %0 = vector.outerproduct %arg0, %arg1, %arg2 {kind = #vector.kind<add>} : vector<4xf32>, vector<8xf32>
    return %0 : vector<4x8xf32>
}

// CHECK-LABEL: func @outerproduct2x8
// CHECK-SAME: %[[A:.*]]: vector<2xf32>, %[[B:.*]]: vector<8xf32>, %[[C:.*]]: vector<2x8xf32>
// CHECK-DAG: %[[C0:.*]] = vector.extract %[[C]][0] : vector<8xf32> from vector<2x8xf32>
// CHECK-DAG: %[[A0:.*]] = vector.scalable.insert %[[A]]{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[A1:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.dupq.lane"(%[[A0]], %c0_i64)
// CHECK-DAG: %[[C1:.*]] = vector.scalable.insert %[[C0]],{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[B0:.*]] = vector.scalable.insert %[[B]],{{.*}} into vector<[4]xf32>
// CHECK: %[[FMLA0:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.fmla.lane"(%[[C1]], %[[B0]], %[[A1]], %c0_i32){{.*}} vector<[4]xf32>
// CHECK-NEXT: %[[R0:.*]] = vector.scalable.extract %[[FMLA0]][0] : vector<8xf32> from vector<[4]xf32>
// CHECK-NEXT: %[[I0:.*]] = vector.insert %[[R0]], %cst [0] : vector<8xf32> into vector<2x8xf32>
// CHECK-DAG: %[[A2:.*]] = vector.scalable.insert %[[A]]{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[A3:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.dupq.lane"(%[[A2]], %c0_i64)
// CHECK-DAG: %[[C2:.*]] = vector.extract %[[C]][1] : vector<8xf32> from vector<2x8xf32>
// CHECK-DAG: %[[C3:.*]] = vector.scalable.insert %[[C2]],{{.*}} into vector<[4]xf32>
// CHECK-DAG: %[[B1:.*]] = vector.scalable.insert %[[B]],{{.*}} into vector<[4]xf32>
// CHECK: %[[FMLA1:.*]] = llvm.call_intrinsic "llvm.aarch64.sve.fmla.lane"(%[[C3]], %[[B1]], %[[A3]], %c1_i32){{.*}} vector<[4]xf32>
// CHECK-NEXT: %[[R1:.*]] = vector.scalable.extract %[[FMLA1]][0] : vector<8xf32> from vector<[4]xf32>
// CHECK-NEXT: %[[I1:.*]] = vector.insert %[[R1]], %[[I0]] [1] : vector<8xf32> into vector<2x8xf32>
// CHECK: return %[[I1]]
func.func @outerproduct2x8(%arg0 : vector<2xf32>, %arg1 : vector<8xf32>, %arg2 : vector<2x8xf32>) ->vector<2x8xf32> {
    %0 = vector.outerproduct %arg0, %arg1, %arg2 {kind = #vector.kind<add>} : vector<2xf32>, vector<8xf32>
    return %0 : vector<2x8xf32>
}

module attributes {transform.with_named_sequence} {
  transform.named_sequence @__transform_main(%arg1: !transform.any_op {transform.readonly}) {
      %f = transform.structured.match ops{["func.func"]} in %arg1 
        : (!transform.any_op) -> !transform.any_op

      transform.apply_patterns to %f {
        transform.apply_patterns.vector.lower_outerproduct enableSVE = true
      } : !transform.any_op
    transform.yield
  }
}

