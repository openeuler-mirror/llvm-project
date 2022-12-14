// RUN: %clang_cc1 -std=c++1y -triple x86_64-linux-gnu -emit-llvm %s -o - | FileCheck %s

struct S {
  S();
  S(S &&);
  ~S();
};

void f() {
  (void) [s(S{})] {};
}

// CHECK-LABEL: define{{.*}} void @_Z1fv(
// CHECK: call void @_ZN1SC1Ev(
// CHECK: call void @"_ZZ1fvEN3$_0D1Ev"(

// CHECK-LABEL: define internal void @"_ZZ1fvEN3$_0D1Ev"(
// CHECK: @"_ZZ1fvEN3$_0D2Ev"(

// D2 at end of file.

void g() {
  [a(1), b(2)] { return a + b; } ();
}

// CHECK-LABEL: define{{.*}} void @_Z1gv(
// CHECK: getelementptr inbounds {{.*}}, i32 0, i32 0
// CHECK: store i32 1, ptr
// CHECK: getelementptr inbounds {{.*}}, i32 0, i32 1
// CHECK: store i32 2, ptr
// CHECK: call noundef i32 @"_ZZ1gvENK3$_1clEv"(

// CHECK-LABEL: define internal noundef i32 @"_ZZ1gvENK3$_1clEv"(
// CHECK: getelementptr inbounds {{.*}}, i32 0, i32 0
// CHECK: load i32, ptr
// CHECK: getelementptr inbounds {{.*}}, i32 0, i32 1
// CHECK: load i32, ptr

// CHECK: add nsw i32

// CHECK-LABEL: define{{.*}} void @_Z18init_capture_dtorsv
void init_capture_dtors() {
  // Ensure that init-captures are not treated as separate full-expressions.
  struct HasDtor { ~HasDtor() {} };
  void some_function_call();
  void other_function_call();
  // CHECK: call {{.*}}some_function_call
  // CHECK: call {{.*}}HasDtorD
  ([x = (HasDtor(), 0)]{}, some_function_call());
  // CHECK: call {{.*}}other_function_call
  other_function_call();
}

int h(int a) {
  // CHECK-LABEL: define{{.*}} i32 @_Z1hi(
  // CHECK: %[[A_ADDR:.*]] = alloca i32,
  // CHECK: %[[OUTER:.*]] = alloca
  // CHECK: store i32 {{.*}}, ptr %[[A_ADDR]],
  //
  // Initialize init-capture 'b(a)' by reference.
  // CHECK: getelementptr inbounds {{.*}}, ptr %[[OUTER]], i32 0, i32 0
  // CHECK: store ptr %[[A_ADDR]], ptr {{.*}},
  //
  // Initialize init-capture 'c(a)' by copy.
  // CHECK: getelementptr inbounds {{.*}}, ptr %[[OUTER]], i32 0, i32 1
  // CHECK: load i32, ptr %[[A_ADDR]],
  // CHECK: store i32
  //
  // CHECK: call noundef i32 @"_ZZ1hiENK3$_2clEv"(ptr {{[^,]*}} %[[OUTER]])
  return [&b(a), c(a)] {
    // CHECK-LABEL: define internal noundef i32 @"_ZZ1hiENK3$_2clEv"(
    // CHECK: %[[OUTER_ADDR:.*]] = alloca
    // CHECK: %[[INNER:.*]] = alloca
    // CHECK: store {{.*}}, ptr %[[OUTER_ADDR]],
    //
    // Capture outer 'c' by reference.
    // CHECK: %[[OUTER:.*]] = load ptr, ptr %[[OUTER_ADDR]]
    // CHECK: getelementptr inbounds {{.*}}, ptr %[[INNER]], i32 0, i32 0
    // CHECK-NEXT: getelementptr inbounds {{.*}}, ptr %[[OUTER]], i32 0, i32 1
    // CHECK-NEXT: store ptr %
    //
    // Capture outer 'b' by copy.
    // CHECK: getelementptr inbounds {{.*}}, ptr %[[INNER]], i32 0, i32 1
    // CHECK-NEXT: getelementptr inbounds {{.*}}, ptr %[[OUTER]], i32 0, i32 0
    // CHECK-NEXT: load ptr, ptr %
    // CHECK-NEXT: load i32, ptr %
    // CHECK-NEXT: store i32
    //
    // CHECK: call noundef i32 @"_ZZZ1hiENK3$_2clEvENKUlvE_clEv"(ptr {{[^,]*}} %[[INNER]])
    return [=, &c] {
      // CHECK-LABEL: define internal void @"_ZZ1fvEN3$_0D2Ev"(
      // CHECK: call void @_ZN1SD1Ev(

      // CHECK-LABEL: define internal noundef i32 @"_ZZZ1hiENK3$_2clEvENKUlvE_clEv"(
      // CHECK: %[[INNER_ADDR:.*]] = alloca
      // CHECK: store {{.*}}, ptr %[[INNER_ADDR]],
      // CHECK: %[[INNER:.*]] = load ptr, ptr %[[INNER_ADDR]]
      //
      // Load capture of 'b'
      // CHECK: getelementptr inbounds {{.*}}, ptr %[[INNER]], i32 0, i32 1
      // CHECK: load i32, ptr %
      //
      // Load capture of 'c'
      // CHECK: getelementptr inbounds {{.*}}, ptr %[[INNER]], i32 0, i32 0
      // CHECK: load ptr, ptr %
      // CHECK: load i32, ptr %
      //
      // CHECK: add nsw i32
      return b + c;
    } ();
  } ();
}

// Ensure we can emit code for init-captures in global lambdas too.
auto global_lambda = [a = 0] () mutable { return ++a; };
int get_incremented() { return global_lambda(); }
