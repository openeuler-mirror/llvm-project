// RUN: mlir-opt -transform-interpreter -canonicalize -cse %s | FileCheck %s 
#map0 = affine_map<(d0, d1)[s0, s1] -> (d0 * s1 + s0 + d1)>
#map1 = affine_map<()[s0] -> (s0 ceildiv 8)>
#map2 = affine_map<(d0, d1, d2) -> (d2, d0)>
#map3 = affine_map<(d0, d1, d2) -> (d2, d1)>
#map4 = affine_map<(d0, d1, d2) -> (d0, d1)>
#map5 = affine_map<(d0) -> (d0 ceildiv 8)>
#map6 = affine_map<(d0, d1) -> (d0 + d1)>

#contraction_accesses_matmul = [
  affine_map<(i, j, k) -> (i, k)>,
  affine_map<(i, j, k) -> (k, j)>,
  affine_map<(i, j, k) -> (i, j)>
]
#contraction_trait_matmul = {
  indexing_maps = #contraction_accesses_matmul,
  iterator_types = ["parallel", "parallel", "reduction"]
}

#contraction_accesses_4d_to_3d = [
  affine_map<(b0, f0, f1, c0, c1) -> (c0, b0, c1, f0)>,
  affine_map<(b0, f0, f1, c0, c1) -> (b0, c1, c0, f1)>,
  affine_map<(b0, f0, f1, c0, c1) -> (b0, f0, f1)>
]
#contraction_trait_4d_to_3d = {
  indexing_maps = #contraction_accesses_4d_to_3d,
  iterator_types = ["parallel", "parallel", "parallel",
                    "reduction", "reduction"]
}

#contraction_accesses_dot = [
affine_map<(i) -> (i)>,
affine_map<(i) -> (i)>,
affine_map<(i) -> ()>
]
#contraction_trait_dot = {
  indexing_maps = #contraction_accesses_dot,
  iterator_types = ["reduction"]
}
#contraction_trait_max = {
  indexing_maps = #contraction_accesses_dot,
  iterator_types = ["reduction"],
  kind = #vector.kind<maxnumf>
}

// CHECK-LABEL: @kernel
module attributes {transform.with_named_sequence} {
  func.func @kernel(%arg0: memref<2048x256xf32, #map0>, %arg1: index, %arg2: memref<256x256x1x8xf32>, %arg3: index, %arg4: memref<32x256x1x8xf32>, %arg5: index) attributes {passthrough = [["prefer-vector-width", "128"]]} {
    %cst = arith.constant 0.000000e+00 : f32
    %c0 = arith.constant 0 : index
    %c256 = arith.constant 256 : index
    %c1 = arith.constant 1 : index
    %0 = affine.apply #map1()[%arg5]
    %1 = vector.transfer_read %arg0[%arg1, %arg5], %cst {in_bounds = [true, true]} : memref<2048x256xf32, #map0>, vector<8x8xf32>
    // CHECK: vector.transfer_read %arg0
    // CHECK: vector.transfer_read %arg0
    // CHECK: vector.transfer_read %arg0
    // CHECK: vector.transfer_read %arg0
    // CHECK: scf.for %[[it0:.*]] = %[[lb:.*]] to %[[ub:.*]] step %[[s:.*]] iter_args(
    // CHECK-SAME: %arg7 = %6, %arg8 = %5, %arg9 = %3, %arg10 = %1) -> (vector<4x4xf32>, vector<4x4xf32>, vector<4x4xf32>, vector<4x4xf32>) {
    %2 = scf.for %arg6 = %c0 to %c256 step %c1 iter_args(%arg7 = %1) -> (vector<8x8xf32>) {
      // CHECK: %[[V0:.*]] = vector.transfer_read %arg2
      // CHECK-NEXT: %[[V1:.*]] = vector.transfer_read %arg2
      // CHECK-NEXT: %[[V2:.*]] = vector.transfer_read %arg4
      // CHECK-NEXT: %[[V3:.*]] = vector.transfer_read %arg4
      %3 = vector.transfer_read %arg2[%arg3, %arg6, %c0, %c0], %cst {in_bounds = [true, true]} : memref<256x256x1x8xf32>, vector<1x8xf32>
      %4 = vector.transfer_read %arg4[%0, %arg6, %c0, %c0], %cst {in_bounds = [true, true]} : memref<32x256x1x8xf32>, vector<1x8xf32>
      //  CHECK: vector.contract {{.*}} %[[V0]], %[[V2]], %arg10 : 
      //  CHECK-NEXT: vector.contract {{.*}} %[[V0]], %[[V3]], %arg9
      //  CHECK-NEXT: vector.contract {{.*}} %[[V1]], %[[V2]], %arg8
      //  CHECK-NEXT: vector.contract {{.*}} %[[V1]], %[[V3]], %arg7
      %5 = vector.contract {indexing_maps = [#map2, #map3, #map4], iterator_types = ["parallel", "parallel", "reduction"], kind = #vector.kind<add>} %3, %4, %arg7 : vector<1x8xf32>, vector<1x8xf32> into vector<8x8xf32>
      // CHECK-NEXT: scf.yield {{.*}}: vector<4x4xf32>, vector<4x4xf32>, vector<4x4xf32>, vector<4x4xf32>
      scf.yield %5 : vector<8x8xf32>
    }
    vector.transfer_write %2, %arg0[%arg1, %arg5] {in_bounds = [true, true]} : vector<8x8xf32>, memref<2048x256xf32, #map0>
    return
  }

  // CHECK-LABEL: @legalizeBinOp
  // CHECK-SAME: (%[[arg0:.*]]: vector<12xf32>, %[[arg1:.*]]: vector<12x6xf32>, %[[arg2:.*]]: vector<6x12x4xf32>, %[[arg3:.*]]: vector<f32>)
  func.func @legalizeBinOp(%arg0 : vector<12xf32>, %arg1 : vector<12x6xf32>, %arg2 : vector<6x12x4xf32>, %arg3 : vector<f32>) -> (vector<12xf32>, vector<6x12xf32>, vector<4x12x6xf32>, vector<f32>) {
    // DCE-ed but does not fail
    %0 = vector.transpose %arg0, [0] : vector<12xf32> to vector<12xf32>
    // CHECK: %[[EX0:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[TR0:.*]] = vector.transpose %[[EX0]]
    // CHECK: %[[EX1:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[TR1:.*]] = vector.transpose %[[EX1]]
    // CHECK: %[[EX2:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[TR2:.*]] = vector.transpose %[[EX2]]
    // CHECK: %[[EX3:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[TR3:.*]] = vector.transpose %[[EX3]]
    // CHECK: %[[EX4:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[TR4:.*]] = vector.transpose %[[EX4]]
    // CHECK: %[[EX5:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[TR5:.*]] = vector.transpose %[[EX5]]
    // CHECK: %[[EX6:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[TR6:.*]] = vector.transpose %[[EX6]]
    // CHECK: %[[EX7:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[TR7:.*]] = vector.transpose %[[EX7]]
    // CHECK: %[[EX8:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[TR8:.*]] = vector.transpose %[[EX8]]
    %1 = vector.transpose %arg1, [1, 0] : vector<12x6xf32> to vector<6x12xf32>

    // Unrolls with factors (2x3x1)
    // CHECK: %[[EX0:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x4x4xf32>
    // CHECK: %[[TR0:.*]] = vector.transpose %[[EX0]]
    // CHECK: %[[EX1:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x4x4xf32>
    // CHECK: %[[TR1:.*]] = vector.transpose %[[EX1]]
    // CHECK: %[[EX2:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x4x4xf32>
    // CHECK: %[[TR2:.*]] = vector.transpose %[[EX2]]
    // CHECK: %[[EX3:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x4x4xf32>
    // CHECK: %[[TR3:.*]] = vector.transpose %[[EX3]]
    // CHECK: %[[EX4:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x4x4xf32>
    // CHECK: %[[TR4:.*]] = vector.transpose %[[EX4]]
    // CHECK: %[[EX5:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x4x4xf32>
    // CHECK: %[[TR5:.*]] = vector.transpose %[[EX5]]
    // CHECK: %[[EX6:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x4x4xf32>
    // CHECK: %[[TR6:.*]] = vector.transpose %[[EX6]]
    // CHECK: %[[EX7:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x4x4xf32>
    // CHECK: %[[TR7:.*]] = vector.transpose %[[EX7]]
    // CHECK: %[[EX8:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x4x4xf32>
    // CHECK: %[[TR8:.*]] = vector.transpose %[[EX8]]
%2 = vector.transpose %arg2, [2, 1, 0] : vector<6x12x4xf32> to vector<4x12x6xf32>

    // CHECK: %[[EX0:.*]] = vector.extract_strided_slice %[[arg0]]{{.*}} to vector<4xf32>
    // CHECK: %[[MUL0:.*]] = arith.mulf %[[EX0]], %[[EX0]] : vector<4xf32>
    // CHECK: %[[EX1:.*]] = vector.extract_strided_slice %[[arg0]]{{.*}} to vector<4xf32>
    // CHECK: %[[MUL1:.*]] = arith.mulf %[[EX1]], %[[EX1]] : vector<4xf32>
    // CHECK: %[[EX2:.*]] = vector.extract_strided_slice %[[arg0]]{{.*}} to vector<4xf32>
    // CHECK: %[[MUL2:.*]] = arith.mulf %[[EX2]], %[[EX2]] : vector<4xf32>
    %res0 = arith.mulf %0, %0: vector<12xf32>
    // CHECK-COUNT-9: arith.mulf{{.*}}: vector<2x4xf32>
    %res1 = arith.mulf %1, %1: vector<6x12xf32>
    // CHECK-COUNT-9: arith.mulf{{.*}}: vector<4x4x2xf32>
    %res2 = arith.mulf %2, %2: vector<4x12x6xf32>
    // CHECK: %[[MUL3:.*]] = arith.mulf %[[arg3]], %[[arg3]] : vector<f32>
    %res3 = arith.mulf %arg3, %arg3: vector<f32>
    return %res0, %res1, %res2, %res3 : vector<12xf32>, vector<6x12xf32>, vector<4x12x6xf32>, vector<f32>
  }

  // CHECK-LABEL: @legalizeContract
  // CHECK-SAME: (%[[arg0:.*]]: vector<4x3xf32>, %[[arg1:.*]]: vector<3x6xf32>, %[[arg2:.*]]: vector<6x3x2x8xf32>
  // CHECK-SAME: %[[arg3:.*]]: vector<3x2x6x2xf32>, %[[arg4:.*]]: vector<8xf32>, %[[arg5:.*]]: vector<8xf16>
  // CHECK-SAME: %[[acc0:.*]]: vector<4x6xf32>, %[[acc1:.*]]: vector<3x8x2xf32>, %[[acc2:.*]]: f32
  func.func @legalizeContract(%arg0 : vector<4x3xf32>, %arg1 : vector<3x6xf32>, %arg2 : vector<6x3x2x8xf32>, %arg3 : vector<3x2x6x2xf32>, %arg4 : vector<8xf32>, %arg5 : vector<8xf16>, %acc0 : vector<4x6xf32>, %acc1 : vector<3x8x2xf32>, %acc2 : f32) -> (vector<4x6xf32>, vector<3x8x2xf32>, f32) {
    // CHECK: %[[EX0:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<3x2xf32>
    // CHECK: %[[EX1:.*]] = vector.extract_strided_slice %[[acc0]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[CO0:.*]] = vector.contract{{.*}} %[[arg0]], %[[EX0]], %[[EX1]]
    // CHECK: %[[EX2:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<3x2xf32>
    // CHECK: %[[EX3:.*]] = vector.extract_strided_slice %[[acc0]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[CO2:.*]] = vector.contract{{.*}} %[[arg0]], %[[EX2]], %[[EX3]]
    // CHECK: %[[EX4:.*]] = vector.extract_strided_slice %[[arg1]]{{.*}} to vector<3x2xf32>
    // CHECK: %[[EX5:.*]] = vector.extract_strided_slice %[[acc0]]{{.*}} to vector<4x2xf32>
    // CHECK: %[[CO3:.*]] = vector.contract{{.*}} %[[arg0]], %[[EX4]], %[[EX5]]
    %0 = vector.contract #contraction_trait_matmul %arg0, %arg1, %acc0 : vector<4x3xf32>, vector<3x6xf32> into vector<4x6xf32>
    // CHECK: %[[EX0:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x3x2x4xf32>
    // CHECK: %[[EX1:.*]] = vector.extract_strided_slice %[[arg3]]{{.*}} to vector<3x2x2x2xf32>
    // CHECK: %[[EX2:.*]] = vector.extract_strided_slice %[[acc1]]{{.*}} to vector<3x4x2xf32>
    // CHECK: %[[CO0:.*]] = vector.contract{{.*}} %[[EX0]], %[[EX1]], %[[EX2]]
    // CHECK: %[[EX3:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x3x2x4xf32>
    // CHECK: %[[EX4:.*]] = vector.extract_strided_slice %[[arg3]]{{.*}} to vector<3x2x2x2xf32>
    // CHECK: %[[CO1:.*]] = vector.contract{{.*}} %[[EX3]], %[[EX4]], %[[CO0]]
    // CHECK: %[[EX5:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x3x2x4xf32>
    // CHECK: %[[EX6:.*]] = vector.extract_strided_slice %[[arg3]]{{.*}} to vector<3x2x2x2xf32>
    // CHECK: %[[CO2:.*]] = vector.contract{{.*}} %[[EX5]], %[[EX6]], %[[CO1]]
    // CHECK: %[[EX7:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x3x2x4xf32>
    // CHECK: %[[EX8:.*]] = vector.extract_strided_slice %[[acc1]]{{.*}} to vector<3x4x2xf32>
    // CHECK: %[[CO2:.*]] = vector.contract{{.*}} %[[EX7]], %[[EX1]], %[[EX8]]
    // CHECK: %[[EX9:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x3x2x4xf32>
    // CHECK: %[[CO3:.*]] = vector.contract{{.*}} %[[EX9]], %[[EX4]], %[[CO2]]
    // CHECK: %[[EX10:.*]] = vector.extract_strided_slice %[[arg2]]{{.*}} to vector<2x3x2x4xf32>
    // CHECK: %[[CO4:.*]] = vector.contract{{.*}} %[[EX10]], %[[EX6]], %[[CO3]]
    %1 = vector.contract #contraction_trait_4d_to_3d %arg2, %arg3, %acc1 : vector<6x3x2x8xf32>, vector<3x2x6x2xf32> into vector<3x8x2xf32>

    // TODO: DOT PRODUCT TYPE CONTRACTION UNROLLING NOT AVAILABLE YET
    // CHECK: %[[CO4:.*]] = vector.contract {{.*}} %[[arg4]], %[[arg4]], %[[acc2]] : vector<8xf32>, vector<8xf32> into f32
    %2 = vector.contract #contraction_trait_dot %arg4, %arg4, %acc2 : vector<8xf32>, vector<8xf32> into f32
    // Vector contraction with mixed typed. lhs/rhs have different element
    // types than accumulator/result.
    // CHECK: %[[CO5:.*]] = vector.contract {{.*}} %[[arg5]], %[[arg5]], %[[CO4]] : vector<8xf16>, vector<8xf16> into f32
    %3 = vector.contract #contraction_trait_dot %arg5, %arg5, %2 : vector<8xf16>, vector<8xf16> into f32
    // CHECK: %[[CO6:.*]] = vector.contract {{.*}} %[[arg4]], %[[arg4]], %[[CO5]] : vector<8xf32>, vector<8xf32> into f32
    %4 = vector.contract #contraction_trait_max %arg4, %arg4, %3 : vector<8xf32>, vector<8xf32> into f32
    return %0, %1, %4 : vector<4x6xf32>, vector<3x8x2xf32>, f32
  }
  transform.named_sequence @__transform_main(%arg1: !transform.any_op {transform.readonly}) {
    transform.legalize
    transform.yield
  }
}