// RUN: split-file %s %t
// RUN: mlir-opt --llvm-legalize-float8-types %t/supported.mlir | FileCheck %s --implicit-check-not=f8E
// RUN: mlir-opt --llvm-legalize-float8-types %t/supported.mlir | mlir-translate --mlir-to-llvmir | FileCheck %s --check-prefix=LLVM
// RUN: mlir-opt --llvm-legalize-float8-types --verify-diagnostics %t/unsupported.mlir | FileCheck %s --check-prefix=PRESERVED

//--- supported.mlir
// Keep the aggregate, extracted vector and shuffle operands in the same type.
// CHECK-LABEL: llvm.func @aggregate_float8
// CHECK: llvm.mlir.constant(dense<0> : vector<2x4xi8>) : !llvm.array<2 x vector<4xi8>>
// CHECK: llvm.extractvalue {{.*}}[0] : !llvm.array<2 x vector<4xi8>>
// CHECK: llvm.mlir.undef : vector<4xi8>
// CHECK: llvm.shufflevector {{.*}} : vector<4xi8>
// CHECK: llvm.insertvalue {{.*}}[1] : !llvm.array<2 x vector<4xi8>>
// LLVM-LABEL: define void @aggregate_float8(
// LLVM: store [2 x <4 x i8>]
llvm.func @aggregate_float8(%dst: !llvm.ptr) {
  %aggregate = llvm.mlir.constant(dense<0.000000e+00> : vector<2x4xf8E5M2>) : !llvm.array<2 x vector<4xf8E5M2>>
  %element = llvm.extractvalue %aggregate[0] : !llvm.array<2 x vector<4xf8E5M2>>
  %other = llvm.mlir.undef : vector<4xf8E5M2>
  %shuffled = llvm.shufflevector %element, %other [0, 1, 2, 3] : vector<4xf8E5M2>
  %updated = llvm.insertvalue %shuffled, %aggregate[1] : !llvm.array<2 x vector<4xf8E5M2>>
  llvm.store %updated, %dst : !llvm.array<2 x vector<4xf8E5M2>>, !llvm.ptr
  llvm.return
}

// Recursion must preserve both array dimensions and the vector shape.
// CHECK-LABEL: llvm.func @nested_array_float8
// CHECK: llvm.mlir.constant(dense<60> : vector<2x3x4xi8>) : !llvm.array<2 x array<3 x vector<4xi8>>>
// CHECK: llvm.extractvalue {{.*}}[1, 2] : !llvm.array<2 x array<3 x vector<4xi8>>>
// CHECK: llvm.insertvalue {{.*}}[0, 1] : !llvm.array<2 x array<3 x vector<4xi8>>>
// LLVM-LABEL: define void @nested_array_float8(
// LLVM: store [2 x [3 x <4 x i8>]]
llvm.func @nested_array_float8(%dst: !llvm.ptr) {
  %aggregate = llvm.mlir.constant(dense<1.000000e+00> : vector<2x3x4xf8E5M2>) : !llvm.array<2 x array<3 x vector<4xf8E5M2>>>
  %element = llvm.extractvalue %aggregate[1, 2] : !llvm.array<2 x array<3 x vector<4xf8E5M2>>>
  %updated = llvm.insertvalue %element, %aggregate[0, 1] : !llvm.array<2 x array<3 x vector<4xf8E5M2>>>
  llvm.store %updated, %dst : !llvm.array<2 x array<3 x vector<4xf8E5M2>>>, !llvm.ptr
  llvm.return
}

// Dense constants must preserve encoding, including a negative zero.
// CHECK-LABEL: llvm.func @constant_bits_float8
// CHECK: llvm.mlir.constant(dense<[60, -64, 56, -128]> : vector<4xi8>) : vector<4xi8>
// LLVM-LABEL: define void @constant_bits_float8(
// LLVM: store <4 x i8> <i8 60, i8 -64, i8 56, i8 -128>
llvm.func @constant_bits_float8(%dst: !llvm.ptr) {
  %value = llvm.mlir.constant(dense<[1.000000e+00, -2.000000e+00, 5.000000e-01, -0.000000e+00]> : vector<4xf8E5M2>) : vector<4xf8E5M2>
  llvm.store %value, %dst : vector<4xf8E5M2>, !llvm.ptr
  llvm.return
}

// Use a second Float8 format to check that conversion is not format-specific.
// CHECK-LABEL: llvm.func @constant_e4m3fn
// CHECK: llvm.mlir.constant(dense<56> : vector<4xi8>) : vector<4xi8>
// LLVM-LABEL: define void @constant_e4m3fn(
// LLVM: store <4 x i8> splat (i8 56)
llvm.func @constant_e4m3fn(%dst: !llvm.ptr) {
  %value = llvm.mlir.constant(dense<1.000000e+00> : vector<4xf8E4M3FN>) : vector<4xf8E4M3FN>
  llvm.store %value, %dst : vector<4xf8E4M3FN>, !llvm.ptr
  llvm.return
}

// Opaque pointer results do not expose alloca's Float8 element attribute.
// CHECK-LABEL: llvm.func @alloca_scalar_float8
// CHECK: llvm.alloca {{.*}} x i8 {alignment = 16 : i64}
// LLVM-LABEL: define ptr @alloca_scalar_float8(
// LLVM: alloca i8, i64 {{.*}}, align 16
llvm.func @alloca_scalar_float8(%size: i64) -> !llvm.ptr {
  %allocation = llvm.alloca %size x f8E5M2 {alignment = 16 : i64} : (i64) -> !llvm.ptr
  llvm.return %allocation : !llvm.ptr
}

// CHECK-LABEL: llvm.func @alloca_nested_float8
// CHECK: llvm.alloca {{.*}} x !llvm.array<2 x array<3 x vector<4xi8>>> {alignment = 32 : i64}
// LLVM-LABEL: define ptr @alloca_nested_float8(
// LLVM: alloca [2 x [3 x <4 x i8>]], i64 {{.*}}, align 32
llvm.func @alloca_nested_float8(%size: i64) -> !llvm.ptr {
  %allocation = llvm.alloca %size x !llvm.array<2 x array<3 x vector<4xf8E5M2>>> {alignment = 32 : i64} : (i64) -> !llvm.ptr
  llvm.return %allocation : !llvm.ptr
}

// GEPs must use the same recursively converted element type as the storage.
// CHECK-LABEL: llvm.func @gep_nested_float8
// CHECK: llvm.getelementptr {{.*}}, !llvm.array<2 x array<3 x vector<4xi8>>>
// LLVM-LABEL: define ptr @gep_nested_float8(
// LLVM: getelementptr [2 x [3 x <4 x i8>]], ptr
llvm.func @gep_nested_float8(%base: !llvm.ptr, %index: i64) -> !llvm.ptr {
  %element = llvm.getelementptr %base[%index, 1, 2, 3] : (!llvm.ptr, i64) -> !llvm.ptr, !llvm.array<2 x array<3 x vector<4xf8E5M2>>>
  llvm.return %element : !llvm.ptr
}

// Array loads, aggregate extraction and bitcasts must agree.
// CHECK-LABEL: llvm.func @load_array_float8
// CHECK: llvm.load {{.*}} : !llvm.ptr -> !llvm.array<2 x vector<4xi8>>
// CHECK: llvm.extractvalue {{.*}}[0] : !llvm.array<2 x vector<4xi8>>
// CHECK: llvm.store {{.*}} : vector<4xi8>, !llvm.ptr
// LLVM-LABEL: define void @load_array_float8(
// LLVM: load [2 x <4 x i8>]
// LLVM: extractvalue [2 x <4 x i8>]
llvm.func @load_array_float8(%src: !llvm.ptr, %dst: !llvm.ptr) {
  %aggregate = llvm.load %src : !llvm.ptr -> !llvm.array<2 x vector<4xf8E5M2>>
  %element = llvm.extractvalue %aggregate[0] : !llvm.array<2 x vector<4xf8E5M2>>
  %bits = llvm.bitcast %element : vector<4xf8E5M2> to vector<4xi8>
  llvm.store %bits, %dst : vector<4xi8>, !llvm.ptr
  llvm.return
}
// A non-splat nested constant must preserve order through storage and subarrays.
// CHECK-LABEL: llvm.func @connected_array_float8
// CHECK: llvm.alloca {{.*}} x !llvm.array<2 x array<2 x vector<2xi8>>>
// CHECK: llvm.mlir.constant(dense<{{.*}}> : vector<2x2x2xi8>) : !llvm.array<2 x array<2 x vector<2xi8>>>
// CHECK: llvm.store {{.*}} : !llvm.array<2 x array<2 x vector<2xi8>>>, !llvm.ptr
// CHECK: llvm.getelementptr {{.*}}, !llvm.array<2 x array<2 x vector<2xi8>>>
// CHECK: llvm.load {{.*}} : !llvm.ptr -> !llvm.array<2 x array<2 x vector<2xi8>>>
// CHECK: llvm.extractvalue {{.*}}[1] : !llvm.array<2 x array<2 x vector<2xi8>>>
// CHECK: llvm.load {{.*}} : !llvm.ptr -> !llvm.array<2 x vector<2xi8>>
// CHECK: llvm.extractvalue {{.*}}[0] : !llvm.array<2 x vector<2xi8>>
// CHECK: llvm.insertvalue {{.*}}[1] : !llvm.array<2 x vector<2xi8>>
// CHECK: llvm.insertvalue {{.*}}[0] : !llvm.array<2 x array<2 x vector<2xi8>>>
// LLVM-LABEL: define void @connected_array_float8(
// LLVM: alloca [2 x [2 x <2 x i8>]]
// LLVM: extractvalue [2 x [2 x <2 x i8>]]
// LLVM: insertvalue [2 x [2 x <2 x i8>]]
llvm.func @connected_array_float8(%dst: !llvm.ptr) {
  %size = llvm.mlir.constant(1 : i64) : i64
  %allocation = llvm.alloca %size x !llvm.array<2 x array<2 x vector<2xf8E5M2>>> {alignment = 16 : i64} : (i64) -> !llvm.ptr
  %value = llvm.mlir.constant(dense<[[[1.0, 2.0], [0.5, -0.0]], [[-1.0, -2.0], [4.0, 8.0]]]> : vector<2x2x2xf8E5M2>) : !llvm.array<2 x array<2 x vector<2xf8E5M2>>>
  llvm.store %value, %allocation : !llvm.array<2 x array<2 x vector<2xf8E5M2>>>, !llvm.ptr
  %second = llvm.getelementptr %allocation[0, 1] : (!llvm.ptr) -> !llvm.ptr, !llvm.array<2 x array<2 x vector<2xf8E5M2>>>
  %whole = llvm.load %allocation : !llvm.ptr -> !llvm.array<2 x array<2 x vector<2xf8E5M2>>>
  %partial = llvm.extractvalue %whole[1] : !llvm.array<2 x array<2 x vector<2xf8E5M2>>>
  %loaded = llvm.load %second : !llvm.ptr -> !llvm.array<2 x vector<2xf8E5M2>>
  %row = llvm.extractvalue %partial[0] : !llvm.array<2 x vector<2xf8E5M2>>
  %updated = llvm.insertvalue %row, %loaded[1] : !llvm.array<2 x vector<2xf8E5M2>>
  %result = llvm.insertvalue %updated, %whole[0] : !llvm.array<2 x array<2 x vector<2xf8E5M2>>>
  llvm.store %result, %dst : !llvm.array<2 x array<2 x vector<2xf8E5M2>>>, !llvm.ptr
  llvm.return
}

// Existing resultless stores follow a producer's storage type through SSA.
// CHECK-LABEL: llvm.func @masked_store_float8
// CHECK: llvm.load {{.*}} : !llvm.ptr -> vector<4xi8>
// CHECK: llvm.intr.masked.store {{.*}} : vector<4xi8>, vector<4xi1> into !llvm.ptr
// LLVM-LABEL: define void @masked_store_float8(
// LLVM: call void @llvm.masked.store.v4i8.p0(
llvm.func @masked_store_float8(%src: !llvm.ptr, %dst: !llvm.ptr, %mask: vector<4xi1>) {
  %value = llvm.load %src : !llvm.ptr -> vector<4xf8E5M2>
  llvm.intr.masked.store %value, %dst, %mask {alignment = 1 : i32} : vector<4xf8E5M2>, vector<4xi1> into !llvm.ptr
  llvm.return
}

// CHECK-LABEL: llvm.func @masked_scatter_float8
// CHECK: llvm.load {{.*}} : !llvm.ptr -> vector<4xi8>
// CHECK: llvm.intr.masked.scatter {{.*}} : vector<4xi8>, vector<4xi1> into !llvm.vec<4 x ptr>
// LLVM-LABEL: define void @masked_scatter_float8(
// LLVM: call void @llvm.masked.scatter.v4i8.v4p0(
llvm.func @masked_scatter_float8(%src: !llvm.ptr, %ptrs: !llvm.vec<4 x ptr>, %mask: vector<4xi1>) {
  %value = llvm.load %src : !llvm.ptr -> vector<4xf8E5M2>
  llvm.intr.masked.scatter %value, %ptrs, %mask {alignment = 1 : i32} : vector<4xf8E5M2>, vector<4xi1> into !llvm.vec<4 x ptr>
  llvm.return
}

// CHECK-LABEL: llvm.func @masked_compressstore_float8
// CHECK: llvm.load {{.*}} : !llvm.ptr -> vector<4xi8>
// CHECK: "llvm.intr.masked.compressstore"({{.*}}) : (vector<4xi8>, !llvm.ptr, vector<4xi1>) -> ()
// LLVM-LABEL: define void @masked_compressstore_float8(
// LLVM: call void @llvm.masked.compressstore.v4i8(
llvm.func @masked_compressstore_float8(%src: !llvm.ptr, %dst: !llvm.ptr, %mask: vector<4xi1>) {
  %value = llvm.load %src : !llvm.ptr -> vector<4xf8E5M2>
  "llvm.intr.masked.compressstore"(%value, %dst, %mask) : (vector<4xf8E5M2>, !llvm.ptr, vector<4xi1>) -> ()
  llvm.return
}

//--- unsupported.mlir
// Unsupported containers must not leave partially rewritten SSA values.
// PRESERVED-LABEL: llvm.func @unsupported_struct_extract
// PRESERVED: llvm.mlir.undef : !llvm.struct<(vector<4xf8E5M2>)>
// PRESERVED: llvm.extractvalue {{.*}} : !llvm.struct<(vector<4xf8E5M2>)>
// PRESERVED: llvm.store {{.*}} : vector<4xf8E5M2>, !llvm.ptr
llvm.func @unsupported_struct_extract(%dst: !llvm.ptr) {
  %container = llvm.mlir.undef : !llvm.struct<(vector<4xf8E5M2>)>
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.extractvalue'}}
  %value = llvm.extractvalue %container[0] : !llvm.struct<(vector<4xf8E5M2>)>
  llvm.store %value, %dst : vector<4xf8E5M2>, !llvm.ptr
  llvm.return
}

// PRESERVED-LABEL: llvm.func @unsupported_struct_insert
// PRESERVED: llvm.mlir.undef : vector<4xf8E5M2>
// PRESERVED: llvm.insertvalue {{.*}} : !llvm.struct<(vector<4xf8E5M2>)>
llvm.func @unsupported_struct_insert() {
  %container = llvm.mlir.undef : !llvm.struct<(vector<4xf8E5M2>)>
  %value = llvm.mlir.undef : vector<4xf8E5M2>
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.insertvalue'}}
  %updated = llvm.insertvalue %value, %container[0] : !llvm.struct<(vector<4xf8E5M2>)>
  llvm.return
}

// A constant must not be retyped while its arithmetic consumer still uses Float8.
// PRESERVED-LABEL: llvm.func @unsupported_constant_arithmetic
// PRESERVED: llvm.mlir.constant(dense<1.000000e+00> : vector<4xf8E5M2>) : vector<4xf8E5M2>
// PRESERVED: arith.cmpf oeq, {{.*}} : vector<4xf8E5M2>
llvm.func @unsupported_constant_arithmetic(%dst: !llvm.ptr) {
  %constant = llvm.mlir.constant(dense<1.0> : vector<4xf8E5M2>) : vector<4xf8E5M2>
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'arith.cmpf'}}
  %comparison = arith.cmpf oeq, %constant, %constant : vector<4xf8E5M2>
  llvm.store %comparison, %dst : vector<4xi1>, !llvm.ptr
  llvm.return
}

// Function arguments retain their declared types and cannot feed retyped arrays.
// PRESERVED-LABEL: llvm.func @unsupported_array_argument
// PRESERVED: llvm.extractvalue {{.*}}[0] : !llvm.array<2 x vector<4xf8E5M2>>
// PRESERVED: llvm.store {{.*}} : vector<4xf8E5M2>, !llvm.ptr
llvm.func @unsupported_array_argument(%array: !llvm.array<2 x vector<4xf8E5M2>>, %dst: !llvm.ptr) {
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.extractvalue'}}
  %value = llvm.extractvalue %array[0] : !llvm.array<2 x vector<4xf8E5M2>>
  llvm.store %value, %dst : vector<4xf8E5M2>, !llvm.ptr
  llvm.return
}

// PRESERVED-LABEL: llvm.func @unsupported_inserted_argument
// PRESERVED: llvm.mlir.undef : !llvm.array<2 x vector<4xf8E5M2>>
// PRESERVED: llvm.insertvalue {{.*}}[0] : !llvm.array<2 x vector<4xf8E5M2>>
llvm.func @unsupported_inserted_argument(%value: vector<4xf8E5M2>, %dst: !llvm.ptr) {
  %array = llvm.mlir.undef : !llvm.array<2 x vector<4xf8E5M2>>
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.insertvalue'}}
  %updated = llvm.insertvalue %value, %array[0] : !llvm.array<2 x vector<4xf8E5M2>>
  llvm.store %updated, %dst : !llvm.array<2 x vector<4xf8E5M2>>, !llvm.ptr
  llvm.return
}

// Follow storage producers so indirect uses of arguments are also preserved.
// PRESERVED-LABEL: llvm.func @unsupported_indirect_argument
// PRESERVED: llvm.shufflevector {{.*}} : vector<4xf8E5M2>
// PRESERVED: llvm.insertvalue {{.*}}[0] : !llvm.array<2 x vector<4xf8E5M2>>
llvm.func @unsupported_indirect_argument(%value: vector<4xf8E5M2>, %dst: !llvm.ptr) {
  %other = llvm.mlir.undef : vector<4xf8E5M2>
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.shufflevector'}}
  %shuffled = llvm.shufflevector %value, %other [0, 1, 2, 3] : vector<4xf8E5M2>
  %array = llvm.mlir.undef : !llvm.array<2 x vector<4xf8E5M2>>
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.insertvalue'}}
  %updated = llvm.insertvalue %shuffled, %array[0] : !llvm.array<2 x vector<4xf8E5M2>>
  llvm.store %updated, %dst : !llvm.array<2 x vector<4xf8E5M2>>, !llvm.ptr
  llvm.return
}

// Integer bitcasts must not hide an unchanged Float8 block argument.
// PRESERVED-LABEL: llvm.func @unsupported_bitcast_roundtrip_argument
// PRESERVED: llvm.bitcast {{.*}} : vector<4xf8E5M2> to vector<4xi8>
// PRESERVED: llvm.bitcast {{.*}} : vector<4xi8> to vector<4xf8E5M2>
// PRESERVED: llvm.mlir.undef : !llvm.array<2 x vector<4xf8E5M2>>
// PRESERVED: llvm.insertvalue {{.*}}[0] : !llvm.array<2 x vector<4xf8E5M2>>
llvm.func @unsupported_bitcast_roundtrip_argument(%value: vector<4xf8E5M2>, %dst: !llvm.ptr) {
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.bitcast'}}
  %bits = llvm.bitcast %value : vector<4xf8E5M2> to vector<4xi8>
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.bitcast'}}
  %fp8 = llvm.bitcast %bits : vector<4xi8> to vector<4xf8E5M2>
  %array = llvm.mlir.undef : !llvm.array<2 x vector<4xf8E5M2>>
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.insertvalue'}}
  %updated = llvm.insertvalue %fp8, %array[0] : !llvm.array<2 x vector<4xf8E5M2>>
  llvm.store %updated, %dst : !llvm.array<2 x vector<4xf8E5M2>>, !llvm.ptr
  llvm.return
}
