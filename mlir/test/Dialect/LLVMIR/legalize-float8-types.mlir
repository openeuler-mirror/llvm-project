// RUN: mlir-opt --llvm-legalize-float8-types %s | FileCheck %s

// Case 1: LoadOp scalar — result type f8 → i8
// CHECK-LABEL: @load_scalar_f8
// CHECK:         llvm.load %{{.*}} : !llvm.ptr -> i8
llvm.func @load_scalar_f8(%src: !llvm.ptr, %dst: !llvm.ptr) {
  %0 = llvm.load %src : !llvm.ptr -> f8E4M3FN
  llvm.store %0, %dst : f8E4M3FN, !llvm.ptr
  llvm.return
}

// Case 2: LoadOp vector — vector<N x f8> → vector<N x i8>
// CHECK-LABEL: @load_vector_f8
// CHECK:         llvm.load %{{.*}} : !llvm.ptr -> vector<8xi8>
llvm.func @load_vector_f8(%src: !llvm.ptr, %dst: !llvm.ptr) {
  %0 = llvm.load %src : !llvm.ptr -> vector<8 x f8E4M3FN>
  llvm.store %0, %dst : vector<8 x f8E4M3FN>, !llvm.ptr
  llvm.return
}

// Case 3: GEPOp — elem_type attribute f8 → i8 (result !llvm.ptr is unchanged)
// CHECK-LABEL: @gep_f8
// CHECK:         llvm.getelementptr %{{.*}}[%{{.*}}] : (!llvm.ptr, i64) -> !llvm.ptr, i8
llvm.func @gep_f8(%base: !llvm.ptr, %idx: i64, %dst: !llvm.ptr) {
  %0 = llvm.getelementptr %base[%idx] : (!llvm.ptr, i64) -> !llvm.ptr, f8E4M3FN
  llvm.store %0, %dst : !llvm.ptr, !llvm.ptr
  llvm.return
}

// Case 4: UndefOp scalar — f8 → i8
// CHECK-LABEL: @undef_scalar_f8
// CHECK:         llvm.mlir.undef : i8
llvm.func @undef_scalar_f8(%dst: !llvm.ptr) {
  %0 = llvm.mlir.undef : f8E4M3FN
  llvm.store %0, %dst : f8E4M3FN, !llvm.ptr
  llvm.return
}

// Case 5: UndefOp vector — vector<N x f8> → vector<N x i8>
// CHECK-LABEL: @undef_vector_f8
// CHECK:         llvm.mlir.undef : vector<4xi8>
llvm.func @undef_vector_f8(%dst: !llvm.ptr) {
  %0 = llvm.mlir.undef : vector<4 x f8E5M2>
  llvm.store %0, %dst : vector<4 x f8E5M2>, !llvm.ptr
  llvm.return
}

// Case 6: InsertElementOp — result vector<N x f8> → vector<N x i8>
// CHECK-LABEL: @insertelement_f8
// CHECK:         llvm.insertelement %{{.*}}, %{{.*}}[%{{.*}} : i32] : vector<4xi8>
llvm.func @insertelement_f8(%ptr: !llvm.ptr, %idx: i32, %dst: !llvm.ptr) {
  %vec = llvm.mlir.undef : vector<4 x f8E4M3FN>
  %val = llvm.load %ptr : !llvm.ptr -> f8E4M3FN
  %0 = llvm.insertelement %val, %vec[%idx : i32] : vector<4 x f8E4M3FN>
  llvm.store %0, %dst : vector<4 x f8E4M3FN>, !llvm.ptr
  llvm.return
}

// Case 7: ShuffleVectorOp — result vector<N x f8> → vector<N x i8>
// CHECK-LABEL: @shufflevector_f8
// CHECK:         llvm.shufflevector %{{.*}}, %{{.*}} [0, 1, 2, 3] : vector<4xi8>
llvm.func @shufflevector_f8(%ptr: !llvm.ptr, %dst: !llvm.ptr) {
  %a = llvm.load %ptr : !llvm.ptr -> vector<4 x f8E4M3FN>
  %b = llvm.mlir.undef : vector<4 x f8E4M3FN>
  %0 = llvm.shufflevector %a, %b [0, 1, 2, 3] : vector<4 x f8E4M3FN>
  llvm.store %0, %dst : vector<4 x f8E4M3FN>, !llvm.ptr
  llvm.return
}

// Case 8: BitcastOp identity — bitcast i8→f8 becomes i8→i8 after legalization;
// the op is eliminated and all uses are replaced by the original i8 argument.
// CHECK-LABEL: @bitcast_identity_f8
// CHECK-NOT:     llvm.bitcast
// CHECK:         llvm.store %arg0, %arg1 : i8, !llvm.ptr
llvm.func @bitcast_identity_f8(%byte: i8, %dst: !llvm.ptr) {
  %0 = llvm.bitcast %byte : i8 to f8E4M3FN
  llvm.store %0, %dst : f8E4M3FN, !llvm.ptr
  llvm.return
}

// Case 9: BitcastOp non-identity — bitcast i32→vector<4 x f8> stays as a
// bitcast but its result type is updated to vector<4 x i8>.
// CHECK-LABEL: @bitcast_non_identity_f8
// CHECK:         llvm.bitcast %{{.*}} : i32 to vector<4xi8>
llvm.func @bitcast_non_identity_f8(%val: i32, %dst: !llvm.ptr) {
  %0 = llvm.bitcast %val : i32 to vector<4 x f8E4M3FN>
  llvm.store %0, %dst : vector<4 x f8E4M3FN>, !llvm.ptr
  llvm.return
}

// Case 10: StoreOp — no explicit handling needed; the value operand's type
// updates implicitly when the defining op's result type is mutated via SSA.
// CHECK-LABEL: @store_implicit_ssa
// CHECK:         %[[V:.*]] = llvm.load %{{.*}} : !llvm.ptr -> i8
// CHECK:         llvm.store %[[V]], %{{.*}} : i8, !llvm.ptr
llvm.func @store_implicit_ssa(%src: !llvm.ptr, %dst: !llvm.ptr) {
  %0 = llvm.load %src : !llvm.ptr -> f8E4M3FN
  llvm.store %0, %dst : f8E4M3FN, !llvm.ptr
  llvm.return
}
