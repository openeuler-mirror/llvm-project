// RUN: mlir-opt --llvm-legalize-float8-types --verify-diagnostics --split-input-file %s | mlir-opt --split-input-file | FileCheck %s
// RUN: mlir-opt --llvm-legalize-float8-types --verify-diagnostics --split-input-file %s | mlir-translate --mlir-to-llvmir --split-input-file | FileCheck %s --check-prefix=LLVM

// Unconverted arguments must not acquire a mismatched vector result type.
// CHECK-LABEL: llvm.func @argument_insertelement
// CHECK: llvm.insertelement {{.*}} : vector<4xf8E5M2>
// CHECK: llvm.store {{.*}} : vector<4xf8E5M2>, !llvm.ptr
// LLVM-LABEL: define void @argument_insertelement(
// LLVM: insertelement <4 x i8>
llvm.func @argument_insertelement(%vec: vector<4xf8E5M2>, %value: f8E5M2, %index: i32, %dst: !llvm.ptr) {
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.insertelement'}}
  %result = llvm.insertelement %value, %vec[%index : i32] : vector<4xf8E5M2>
  llvm.store %result, %dst : vector<4xf8E5M2>, !llvm.ptr
  llvm.return
}

// -----

// Reparse the result to catch a shuffle whose printed result type is inferred.
// CHECK-LABEL: llvm.func @argument_shuffle
// CHECK: llvm.shufflevector {{.*}} : vector<4xf8E5M2>
// CHECK: llvm.store {{.*}} : vector<4xf8E5M2>, !llvm.ptr
// LLVM-LABEL: define void @argument_shuffle(
// LLVM: store <4 x i8>
llvm.func @argument_shuffle(%vec: vector<4xf8E5M2>, %dst: !llvm.ptr) {
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.shufflevector'}}
  %result = llvm.shufflevector %vec, %vec [0, 1, 2, 3] : vector<4xf8E5M2>
  llvm.store %result, %dst : vector<4xf8E5M2>, !llvm.ptr
  llvm.return
}

// -----

// Folding a bitcast must not pass an unchanged Float8 argument to integer ops.
// CHECK-LABEL: llvm.func @argument_bitcast_integer
// CHECK: llvm.bitcast {{.*}} : vector<4xf8E5M2> to vector<4xi8>
// CHECK: llvm.add {{.*}} : vector<4xi8>
// LLVM-LABEL: define void @argument_bitcast_integer(
// LLVM: add <4 x i8>
llvm.func @argument_bitcast_integer(%vec: vector<4xf8E5M2>, %dst: !llvm.ptr) {
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.bitcast'}}
  %bits = llvm.bitcast %vec : vector<4xf8E5M2> to vector<4xi8>
  %sum = llvm.add %bits, %bits : vector<4xi8>
  llvm.store %sum, %dst : vector<4xi8>, !llvm.ptr
  llvm.return
}

// -----

// Rejection must precede any otherwise-supported storage rewrite in the module.
// CHECK-LABEL: llvm.func @argument_bitcast_store
// CHECK: llvm.bitcast {{.*}} : vector<4xf8E5M2> to vector<4xi8>
// CHECK: llvm.store {{.*}} : vector<4xi8>, !llvm.ptr
// CHECK-LABEL: llvm.func @preserved_storage
// CHECK: llvm.load {{.*}} : !llvm.ptr -> vector<4xf8E5M2>
// LLVM-LABEL: define void @argument_bitcast_store(
// LLVM: store <4 x i8>
// LLVM-LABEL: define void @preserved_storage(
// LLVM: load <4 x i8>
llvm.func @argument_bitcast_store(%vec: vector<4xf8E5M2>, %dst: !llvm.ptr) {
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.bitcast'}}
  %bits = llvm.bitcast %vec : vector<4xf8E5M2> to vector<4xi8>
  llvm.store %bits, %dst : vector<4xi8>, !llvm.ptr
  llvm.return
}
llvm.func @preserved_storage(%src: !llvm.ptr, %dst: !llvm.ptr) {
  %value = llvm.load %src : !llvm.ptr -> vector<4xf8E5M2>
  llvm.store %value, %dst : vector<4xf8E5M2>, !llvm.ptr
  llvm.return
}

// -----

// Stores follow their SSA value types; direct arguments need no rewrite.
// CHECK-LABEL: llvm.func @argument_stores
// CHECK: llvm.store {{.*}} : vector<4xf8E5M2>, !llvm.ptr
// CHECK: llvm.intr.masked.store {{.*}} : vector<4xf8E5M2>, vector<4xi1> into !llvm.ptr
// CHECK: llvm.intr.masked.scatter {{.*}} : vector<4xf8E5M2>, vector<4xi1> into !llvm.vec<4 x ptr>
// CHECK: "llvm.intr.masked.compressstore"({{.*}}) : (vector<4xf8E5M2>, !llvm.ptr, vector<4xi1>) -> ()
// CHECK-LABEL: llvm.func @converted_storage
// CHECK: llvm.load {{.*}} : !llvm.ptr -> vector<4xi8>
// CHECK: llvm.store {{.*}} : vector<4xi8>, !llvm.ptr
// LLVM-LABEL: define void @argument_stores(
// LLVM: store <4 x i8>
// LLVM: call void @llvm.masked.store.v4i8.p0(
// LLVM: call void @llvm.masked.scatter.v4i8.v4p0(
// LLVM: call void @llvm.masked.compressstore.v4i8(
// LLVM-LABEL: define void @converted_storage(
// LLVM: load <4 x i8>
llvm.func @argument_stores(%value: vector<4xf8E5M2>, %dst: !llvm.ptr, %ptrs: !llvm.vec<4 x ptr>, %mask: vector<4xi1>) {
  llvm.store %value, %dst : vector<4xf8E5M2>, !llvm.ptr
  llvm.intr.masked.store %value, %dst, %mask {alignment = 1 : i32} : vector<4xf8E5M2>, vector<4xi1> into !llvm.ptr
  llvm.intr.masked.scatter %value, %ptrs, %mask {alignment = 1 : i32} : vector<4xf8E5M2>, vector<4xi1> into !llvm.vec<4 x ptr>
  "llvm.intr.masked.compressstore"(%value, %dst, %mask) : (vector<4xf8E5M2>, !llvm.ptr, vector<4xi1>) -> ()
  llvm.return
}
llvm.func @converted_storage(%src: !llvm.ptr, %dst: !llvm.ptr) {
  %value = llvm.load %src : !llvm.ptr -> vector<4xf8E5M2>
  llvm.store %value, %dst : vector<4xf8E5M2>, !llvm.ptr
  llvm.return
}

// -----

// Preserve the established masked-load passthrough representation for export.
// CHECK-LABEL: llvm.func @argument_maskedload
// CHECK: llvm.intr.masked.load {{.*}} (!llvm.ptr, vector<4xi1>, vector<4xf8E5M2>) -> vector<4xi8>
// LLVM-LABEL: define void @argument_maskedload(
// LLVM: call <4 x i8> @llvm.masked.load.v4i8.p0(
llvm.func @argument_maskedload(%src: !llvm.ptr, %mask: vector<4xi1>, %pass: vector<4xf8E5M2>, %dst: !llvm.ptr) {
  %value = llvm.intr.masked.load %src, %mask, %pass {alignment = 1 : i32} : (!llvm.ptr, vector<4xi1>, vector<4xf8E5M2>) -> vector<4xf8E5M2>
  llvm.store %value, %dst : vector<4xf8E5M2>, !llvm.ptr
  llvm.return
}

// -----

// A call boundary must reject the scope before changing the load's aggregate.
// CHECK: llvm.func @use(!llvm.array<2 x vector<4xf8E5M2>>)
// CHECK-LABEL: llvm.func @aggregate_call
// CHECK: llvm.load {{.*}} : !llvm.ptr -> !llvm.array<2 x vector<4xf8E5M2>>
// CHECK: llvm.call @use({{.*}}) : (!llvm.array<2 x vector<4xf8E5M2>>) -> ()
// LLVM: declare void @use([2 x <4 x i8>])
// LLVM-LABEL: define void @aggregate_call(
// LLVM: call void @use([2 x <4 x i8>]
llvm.func @use(!llvm.array<2 x vector<4xf8E5M2>>)
llvm.func @aggregate_call(%src: !llvm.ptr) {
  %value = llvm.load %src : !llvm.ptr -> !llvm.array<2 x vector<4xf8E5M2>>
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.call'}}
  llvm.call @use(%value) : (!llvm.array<2 x vector<4xf8E5M2>>) -> ()
  llvm.return
}

// -----

// CHECK-LABEL: llvm.func @aggregate_return
// CHECK: llvm.load {{.*}} : !llvm.ptr -> !llvm.array<2 x vector<4xf8E5M2>>
// CHECK: llvm.return {{.*}} : !llvm.array<2 x vector<4xf8E5M2>>
// LLVM-LABEL: define [2 x <4 x i8>] @aggregate_return(
// LLVM: ret [2 x <4 x i8>]
llvm.func @aggregate_return(%src: !llvm.ptr) -> !llvm.array<2 x vector<4xf8E5M2>> {
  %value = llvm.load %src : !llvm.ptr -> !llvm.array<2 x vector<4xf8E5M2>>
  // expected-warning@+1 {{LegalizeFloat8Types: unexpected Float8 type on op 'llvm.return'}}
  llvm.return %value : !llvm.array<2 x vector<4xf8E5M2>>
}
