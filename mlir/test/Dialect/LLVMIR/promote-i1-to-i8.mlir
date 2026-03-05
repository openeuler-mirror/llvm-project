// RUN: mlir-opt --llvm-promote-i1-to-i8 %s | FileCheck %s

// -----

// Case 1: Store vector<N x i1> — zext to vector<N x i8> before store.
// CHECK-LABEL: @store_vector_i1
// CHECK:         %[[ZEXT:.*]] = llvm.zext %{{.*}} : vector<128xi1> to vector<128xi8>
// CHECK-NEXT:    llvm.store %[[ZEXT]], %{{.*}} : vector<128xi8>, !llvm.ptr
llvm.func @store_vector_i1(%val: vector<128xi1>, %ptr: !llvm.ptr) {
  llvm.store %val, %ptr : vector<128xi1>, !llvm.ptr
  llvm.return
}

// -----

// Case 2: Load vector<N x i1> — load as vector<N x i8> then trunc.
// CHECK-LABEL: @load_vector_i1
// CHECK:         %[[LOAD:.*]] = llvm.load %{{.*}} : !llvm.ptr -> vector<128xi8>
// CHECK-NEXT:    %[[TRUNC:.*]] = llvm.trunc %[[LOAD]] : vector<128xi8> to vector<128xi1>
// CHECK-NEXT:    llvm.return %[[TRUNC]]
llvm.func @load_vector_i1(%ptr: !llvm.ptr) -> vector<128xi1> {
  %0 = llvm.load %ptr : !llvm.ptr -> vector<128xi1>
  llvm.return %0 : vector<128xi1>
}

// -----

// Case 3: Store with alignment preserved.
// CHECK-LABEL: @store_vector_i1_aligned
// CHECK:         %[[ZEXT:.*]] = llvm.zext %{{.*}} : vector<64xi1> to vector<64xi8>
// CHECK-NEXT:    llvm.store %[[ZEXT]], %{{.*}} {alignment = 64 : i64} : vector<64xi8>, !llvm.ptr
llvm.func @store_vector_i1_aligned(%val: vector<64xi1>, %ptr: !llvm.ptr) {
  llvm.store %val, %ptr {alignment = 64 : i64} : vector<64xi1>, !llvm.ptr
  llvm.return
}

// -----

// Case 4: Load with alignment preserved.
// CHECK-LABEL: @load_vector_i1_aligned
// CHECK:         %[[LOAD:.*]] = llvm.load %{{.*}} {alignment = 64 : i64} : !llvm.ptr -> vector<64xi8>
// CHECK-NEXT:    %[[TRUNC:.*]] = llvm.trunc %[[LOAD]] : vector<64xi8> to vector<64xi1>
llvm.func @load_vector_i1_aligned(%ptr: !llvm.ptr) -> vector<64xi1> {
  %0 = llvm.load %ptr {alignment = 64 : i64} : !llvm.ptr -> vector<64xi1>
  llvm.return %0 : vector<64xi1>
}

// -----

// Case 5: Scalar i1 store — should NOT be promoted (only vectors).
// CHECK-LABEL: @store_scalar_i1
// CHECK:         llvm.store %{{.*}}, %{{.*}} : i1, !llvm.ptr
// CHECK-NOT:     llvm.zext
llvm.func @store_scalar_i1(%val: i1, %ptr: !llvm.ptr) {
  llvm.store %val, %ptr : i1, !llvm.ptr
  llvm.return
}

// -----

// Case 6: Scalar i1 load — should NOT be promoted (only vectors).
// CHECK-LABEL: @load_scalar_i1
// CHECK:         llvm.load %{{.*}} : !llvm.ptr -> i1
// CHECK-NOT:     llvm.trunc
llvm.func @load_scalar_i1(%ptr: !llvm.ptr) -> i1 {
  %0 = llvm.load %ptr : !llvm.ptr -> i1
  llvm.return %0 : i1
}

// -----

// Case 7: Non-i1 vector store — should NOT be promoted.
// CHECK-LABEL: @store_vector_i32
// CHECK:         llvm.store %{{.*}}, %{{.*}} : vector<4xi32>, !llvm.ptr
// CHECK-NOT:     llvm.zext
llvm.func @store_vector_i32(%val: vector<4xi32>, %ptr: !llvm.ptr) {
  llvm.store %val, %ptr : vector<4xi32>, !llvm.ptr
  llvm.return
}

// -----

// Case 8: Round-trip — store then load vector<N x i1>.
// CHECK-LABEL: @roundtrip_vector_i1
// CHECK:         %[[ZEXT:.*]] = llvm.zext %{{.*}} : vector<16xi1> to vector<16xi8>
// CHECK-NEXT:    llvm.store %[[ZEXT]], %{{.*}} : vector<16xi8>, !llvm.ptr
// CHECK-NEXT:    %[[LOAD:.*]] = llvm.load %{{.*}} : !llvm.ptr -> vector<16xi8>
// CHECK-NEXT:    %[[TRUNC:.*]] = llvm.trunc %[[LOAD]] : vector<16xi8> to vector<16xi1>
llvm.func @roundtrip_vector_i1(%val: vector<16xi1>, %ptr: !llvm.ptr) -> vector<16xi1> {
  llvm.store %val, %ptr : vector<16xi1>, !llvm.ptr
  %0 = llvm.load %ptr : !llvm.ptr -> vector<16xi1>
  llvm.return %0 : vector<16xi1>
}
