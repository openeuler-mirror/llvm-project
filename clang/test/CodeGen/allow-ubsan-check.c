// NOTE: Assertions have been autogenerated by utils/update_cc_test_checks.py UTC_ARGS: --version 4
// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -emit-llvm -o - %s -fsanitize=signed-integer-overflow,integer-divide-by-zero,null -mllvm -ubsan-guard-checks | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -emit-llvm -o - %s -fsanitize=signed-integer-overflow,integer-divide-by-zero,null -mllvm -ubsan-guard-checks -fsanitize-trap=signed-integer-overflow,integer-divide-by-zero,null | FileCheck %s --check-prefixes=TR
// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -emit-llvm -o - %s -fsanitize=signed-integer-overflow,integer-divide-by-zero,null -mllvm -ubsan-guard-checks -fsanitize-recover=signed-integer-overflow,integer-divide-by-zero,null | FileCheck %s --check-prefixes=REC


// CHECK-LABEL: define dso_local i32 @div(
// CHECK-SAME: i32 noundef [[X:%.*]], i32 noundef [[Y:%.*]]) #[[ATTR0:[0-9]+]] {
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[X_ADDR:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[Y_ADDR:%.*]] = alloca i32, align 4
// CHECK-NEXT:    store i32 [[X]], ptr [[X_ADDR]], align 4
// CHECK-NEXT:    store i32 [[Y]], ptr [[Y_ADDR]], align 4
// CHECK-NEXT:    [[TMP0:%.*]] = load i32, ptr [[X_ADDR]], align 4
// CHECK-NEXT:    [[TMP1:%.*]] = load i32, ptr [[Y_ADDR]], align 4
// CHECK-NEXT:    [[TMP2:%.*]] = icmp ne i32 [[TMP1]], 0, !nosanitize [[META2:![0-9]+]]
// CHECK-NEXT:    [[TMP3:%.*]] = icmp ne i32 [[TMP0]], -2147483648, !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP4:%.*]] = icmp ne i32 [[TMP1]], -1, !nosanitize [[META2]]
// CHECK-NEXT:    [[OR:%.*]] = or i1 [[TMP3]], [[TMP4]], !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP5:%.*]] = and i1 [[TMP2]], [[OR]], !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP6:%.*]] = call i1 @llvm.allow.ubsan.check(i8 3), !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP7:%.*]] = xor i1 [[TMP6]], true, !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP8:%.*]] = or i1 [[TMP5]], [[TMP7]], !nosanitize [[META2]]
// CHECK-NEXT:    br i1 [[TMP8]], label [[CONT:%.*]], label [[HANDLER_DIVREM_OVERFLOW:%.*]], !prof [[PROF3:![0-9]+]], !nosanitize [[META2]]
// CHECK:       handler.divrem_overflow:
// CHECK-NEXT:    [[TMP9:%.*]] = zext i32 [[TMP0]] to i64, !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP10:%.*]] = zext i32 [[TMP1]] to i64, !nosanitize [[META2]]
// CHECK-NEXT:    call void @__ubsan_handle_divrem_overflow_abort(ptr @[[GLOB1:[0-9]+]], i64 [[TMP9]], i64 [[TMP10]]) #[[ATTR4:[0-9]+]], !nosanitize [[META2]]
// CHECK-NEXT:    unreachable, !nosanitize [[META2]]
// CHECK:       cont:
// CHECK-NEXT:    [[DIV:%.*]] = sdiv i32 [[TMP0]], [[TMP1]]
// CHECK-NEXT:    ret i32 [[DIV]]
//
// TR-LABEL: define dso_local i32 @div(
// TR-SAME: i32 noundef [[X:%.*]], i32 noundef [[Y:%.*]]) #[[ATTR0:[0-9]+]] {
// TR-NEXT:  entry:
// TR-NEXT:    [[X_ADDR:%.*]] = alloca i32, align 4
// TR-NEXT:    [[Y_ADDR:%.*]] = alloca i32, align 4
// TR-NEXT:    store i32 [[X]], ptr [[X_ADDR]], align 4
// TR-NEXT:    store i32 [[Y]], ptr [[Y_ADDR]], align 4
// TR-NEXT:    [[TMP0:%.*]] = load i32, ptr [[X_ADDR]], align 4
// TR-NEXT:    [[TMP1:%.*]] = load i32, ptr [[Y_ADDR]], align 4
// TR-NEXT:    [[TMP2:%.*]] = icmp ne i32 [[TMP1]], 0, !nosanitize [[META2:![0-9]+]]
// TR-NEXT:    [[TMP3:%.*]] = icmp ne i32 [[TMP0]], -2147483648, !nosanitize [[META2]]
// TR-NEXT:    [[TMP4:%.*]] = icmp ne i32 [[TMP1]], -1, !nosanitize [[META2]]
// TR-NEXT:    [[OR:%.*]] = or i1 [[TMP3]], [[TMP4]], !nosanitize [[META2]]
// TR-NEXT:    [[TMP5:%.*]] = and i1 [[TMP2]], [[OR]], !nosanitize [[META2]]
// TR-NEXT:    [[TMP6:%.*]] = call i1 @llvm.allow.ubsan.check(i8 3), !nosanitize [[META2]]
// TR-NEXT:    [[TMP7:%.*]] = xor i1 [[TMP6]], true, !nosanitize [[META2]]
// TR-NEXT:    [[TMP8:%.*]] = or i1 [[TMP5]], [[TMP7]], !nosanitize [[META2]]
// TR-NEXT:    br i1 [[TMP8]], label [[CONT:%.*]], label [[TRAP:%.*]], !nosanitize [[META2]]
// TR:       trap:
// TR-NEXT:    call void @llvm.ubsantrap(i8 3) #[[ATTR4:[0-9]+]], !nosanitize [[META2]]
// TR-NEXT:    unreachable, !nosanitize [[META2]]
// TR:       cont:
// TR-NEXT:    [[DIV:%.*]] = sdiv i32 [[TMP0]], [[TMP1]]
// TR-NEXT:    ret i32 [[DIV]]
//
// REC-LABEL: define dso_local i32 @div(
// REC-SAME: i32 noundef [[X:%.*]], i32 noundef [[Y:%.*]]) #[[ATTR0:[0-9]+]] {
// REC-NEXT:  entry:
// REC-NEXT:    [[X_ADDR:%.*]] = alloca i32, align 4
// REC-NEXT:    [[Y_ADDR:%.*]] = alloca i32, align 4
// REC-NEXT:    store i32 [[X]], ptr [[X_ADDR]], align 4
// REC-NEXT:    store i32 [[Y]], ptr [[Y_ADDR]], align 4
// REC-NEXT:    [[TMP0:%.*]] = load i32, ptr [[X_ADDR]], align 4
// REC-NEXT:    [[TMP1:%.*]] = load i32, ptr [[Y_ADDR]], align 4
// REC-NEXT:    [[TMP2:%.*]] = icmp ne i32 [[TMP1]], 0, !nosanitize [[META2:![0-9]+]]
// REC-NEXT:    [[TMP3:%.*]] = icmp ne i32 [[TMP0]], -2147483648, !nosanitize [[META2]]
// REC-NEXT:    [[TMP4:%.*]] = icmp ne i32 [[TMP1]], -1, !nosanitize [[META2]]
// REC-NEXT:    [[OR:%.*]] = or i1 [[TMP3]], [[TMP4]], !nosanitize [[META2]]
// REC-NEXT:    [[TMP5:%.*]] = and i1 [[TMP2]], [[OR]], !nosanitize [[META2]]
// REC-NEXT:    [[TMP6:%.*]] = call i1 @llvm.allow.ubsan.check(i8 3), !nosanitize [[META2]]
// REC-NEXT:    [[TMP7:%.*]] = xor i1 [[TMP6]], true, !nosanitize [[META2]]
// REC-NEXT:    [[TMP8:%.*]] = or i1 [[TMP5]], [[TMP7]], !nosanitize [[META2]]
// REC-NEXT:    br i1 [[TMP8]], label [[CONT:%.*]], label [[HANDLER_DIVREM_OVERFLOW:%.*]], !prof [[PROF3:![0-9]+]], !nosanitize [[META2]]
// REC:       handler.divrem_overflow:
// REC-NEXT:    [[TMP9:%.*]] = zext i32 [[TMP0]] to i64, !nosanitize [[META2]]
// REC-NEXT:    [[TMP10:%.*]] = zext i32 [[TMP1]] to i64, !nosanitize [[META2]]
// REC-NEXT:    call void @__ubsan_handle_divrem_overflow(ptr @[[GLOB1:[0-9]+]], i64 [[TMP9]], i64 [[TMP10]]) #[[ATTR4:[0-9]+]], !nosanitize [[META2]]
// REC-NEXT:    br label [[CONT]], !nosanitize [[META2]]
// REC:       cont:
// REC-NEXT:    [[DIV:%.*]] = sdiv i32 [[TMP0]], [[TMP1]]
// REC-NEXT:    ret i32 [[DIV]]
//
int div(int x, int y) {
  return x / y;
}

// CHECK-LABEL: define dso_local i32 @null(
// CHECK-SAME: ptr noundef [[X:%.*]]) #[[ATTR0]] {
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[X_ADDR:%.*]] = alloca ptr, align 8
// CHECK-NEXT:    store ptr [[X]], ptr [[X_ADDR]], align 8
// CHECK-NEXT:    [[TMP0:%.*]] = load ptr, ptr [[X_ADDR]], align 8
// CHECK-NEXT:    [[TMP1:%.*]] = icmp ne ptr [[TMP0]], null, !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP2:%.*]] = call i1 @llvm.allow.ubsan.check(i8 22), !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP3:%.*]] = xor i1 [[TMP2]], true, !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP4:%.*]] = or i1 [[TMP1]], [[TMP3]], !nosanitize [[META2]]
// CHECK-NEXT:    br i1 [[TMP4]], label [[CONT:%.*]], label [[HANDLER_TYPE_MISMATCH:%.*]], !prof [[PROF3]], !nosanitize [[META2]]
// CHECK:       handler.type_mismatch:
// CHECK-NEXT:    [[TMP5:%.*]] = ptrtoint ptr [[TMP0]] to i64, !nosanitize [[META2]]
// CHECK-NEXT:    call void @__ubsan_handle_type_mismatch_v1_abort(ptr @[[GLOB2:[0-9]+]], i64 [[TMP5]]) #[[ATTR4]], !nosanitize [[META2]]
// CHECK-NEXT:    unreachable, !nosanitize [[META2]]
// CHECK:       cont:
// CHECK-NEXT:    [[TMP6:%.*]] = load i32, ptr [[TMP0]], align 4
// CHECK-NEXT:    ret i32 [[TMP6]]
//
// TR-LABEL: define dso_local i32 @null(
// TR-SAME: ptr noundef [[X:%.*]]) #[[ATTR0]] {
// TR-NEXT:  entry:
// TR-NEXT:    [[X_ADDR:%.*]] = alloca ptr, align 8
// TR-NEXT:    store ptr [[X]], ptr [[X_ADDR]], align 8
// TR-NEXT:    [[TMP0:%.*]] = load ptr, ptr [[X_ADDR]], align 8
// TR-NEXT:    [[TMP1:%.*]] = icmp ne ptr [[TMP0]], null, !nosanitize [[META2]]
// TR-NEXT:    [[TMP2:%.*]] = call i1 @llvm.allow.ubsan.check(i8 22), !nosanitize [[META2]]
// TR-NEXT:    [[TMP3:%.*]] = xor i1 [[TMP2]], true, !nosanitize [[META2]]
// TR-NEXT:    [[TMP4:%.*]] = or i1 [[TMP1]], [[TMP3]], !nosanitize [[META2]]
// TR-NEXT:    br i1 [[TMP4]], label [[CONT:%.*]], label [[TRAP:%.*]], !nosanitize [[META2]]
// TR:       trap:
// TR-NEXT:    call void @llvm.ubsantrap(i8 22) #[[ATTR4]], !nosanitize [[META2]]
// TR-NEXT:    unreachable, !nosanitize [[META2]]
// TR:       cont:
// TR-NEXT:    [[TMP5:%.*]] = load i32, ptr [[TMP0]], align 4
// TR-NEXT:    ret i32 [[TMP5]]
//
// REC-LABEL: define dso_local i32 @null(
// REC-SAME: ptr noundef [[X:%.*]]) #[[ATTR0]] {
// REC-NEXT:  entry:
// REC-NEXT:    [[X_ADDR:%.*]] = alloca ptr, align 8
// REC-NEXT:    store ptr [[X]], ptr [[X_ADDR]], align 8
// REC-NEXT:    [[TMP0:%.*]] = load ptr, ptr [[X_ADDR]], align 8
// REC-NEXT:    [[TMP1:%.*]] = icmp ne ptr [[TMP0]], null, !nosanitize [[META2]]
// REC-NEXT:    [[TMP2:%.*]] = call i1 @llvm.allow.ubsan.check(i8 22), !nosanitize [[META2]]
// REC-NEXT:    [[TMP3:%.*]] = xor i1 [[TMP2]], true, !nosanitize [[META2]]
// REC-NEXT:    [[TMP4:%.*]] = or i1 [[TMP1]], [[TMP3]], !nosanitize [[META2]]
// REC-NEXT:    br i1 [[TMP4]], label [[CONT:%.*]], label [[HANDLER_TYPE_MISMATCH:%.*]], !prof [[PROF3]], !nosanitize [[META2]]
// REC:       handler.type_mismatch:
// REC-NEXT:    [[TMP5:%.*]] = ptrtoint ptr [[TMP0]] to i64, !nosanitize [[META2]]
// REC-NEXT:    call void @__ubsan_handle_type_mismatch_v1(ptr @[[GLOB2:[0-9]+]], i64 [[TMP5]]) #[[ATTR4]], !nosanitize [[META2]]
// REC-NEXT:    br label [[CONT]], !nosanitize [[META2]]
// REC:       cont:
// REC-NEXT:    [[TMP6:%.*]] = load i32, ptr [[TMP0]], align 4
// REC-NEXT:    ret i32 [[TMP6]]
//
int null(int* x) {
  return *x;
}

// CHECK-LABEL: define dso_local i32 @overflow(
// CHECK-SAME: i32 noundef [[X:%.*]], i32 noundef [[Y:%.*]]) #[[ATTR0]] {
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[X_ADDR:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[Y_ADDR:%.*]] = alloca i32, align 4
// CHECK-NEXT:    store i32 [[X]], ptr [[X_ADDR]], align 4
// CHECK-NEXT:    store i32 [[Y]], ptr [[Y_ADDR]], align 4
// CHECK-NEXT:    [[TMP0:%.*]] = load i32, ptr [[X_ADDR]], align 4
// CHECK-NEXT:    [[TMP1:%.*]] = load i32, ptr [[Y_ADDR]], align 4
// CHECK-NEXT:    [[TMP2:%.*]] = call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 [[TMP0]], i32 [[TMP1]]), !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP3:%.*]] = extractvalue { i32, i1 } [[TMP2]], 0, !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP4:%.*]] = extractvalue { i32, i1 } [[TMP2]], 1, !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP5:%.*]] = xor i1 [[TMP4]], true, !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP6:%.*]] = call i1 @llvm.allow.ubsan.check(i8 0), !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP7:%.*]] = xor i1 [[TMP6]], true, !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP8:%.*]] = or i1 [[TMP5]], [[TMP7]], !nosanitize [[META2]]
// CHECK-NEXT:    br i1 [[TMP8]], label [[CONT:%.*]], label [[HANDLER_ADD_OVERFLOW:%.*]], !prof [[PROF3]], !nosanitize [[META2]]
// CHECK:       handler.add_overflow:
// CHECK-NEXT:    [[TMP9:%.*]] = zext i32 [[TMP0]] to i64, !nosanitize [[META2]]
// CHECK-NEXT:    [[TMP10:%.*]] = zext i32 [[TMP1]] to i64, !nosanitize [[META2]]
// CHECK-NEXT:    call void @__ubsan_handle_add_overflow_abort(ptr @[[GLOB3:[0-9]+]], i64 [[TMP9]], i64 [[TMP10]]) #[[ATTR4]], !nosanitize [[META2]]
// CHECK-NEXT:    unreachable, !nosanitize [[META2]]
// CHECK:       cont:
// CHECK-NEXT:    ret i32 [[TMP3]]
//
// TR-LABEL: define dso_local i32 @overflow(
// TR-SAME: i32 noundef [[X:%.*]], i32 noundef [[Y:%.*]]) #[[ATTR0]] {
// TR-NEXT:  entry:
// TR-NEXT:    [[X_ADDR:%.*]] = alloca i32, align 4
// TR-NEXT:    [[Y_ADDR:%.*]] = alloca i32, align 4
// TR-NEXT:    store i32 [[X]], ptr [[X_ADDR]], align 4
// TR-NEXT:    store i32 [[Y]], ptr [[Y_ADDR]], align 4
// TR-NEXT:    [[TMP0:%.*]] = load i32, ptr [[X_ADDR]], align 4
// TR-NEXT:    [[TMP1:%.*]] = load i32, ptr [[Y_ADDR]], align 4
// TR-NEXT:    [[TMP2:%.*]] = call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 [[TMP0]], i32 [[TMP1]]), !nosanitize [[META2]]
// TR-NEXT:    [[TMP3:%.*]] = extractvalue { i32, i1 } [[TMP2]], 0, !nosanitize [[META2]]
// TR-NEXT:    [[TMP4:%.*]] = extractvalue { i32, i1 } [[TMP2]], 1, !nosanitize [[META2]]
// TR-NEXT:    [[TMP5:%.*]] = xor i1 [[TMP4]], true, !nosanitize [[META2]]
// TR-NEXT:    [[TMP6:%.*]] = call i1 @llvm.allow.ubsan.check(i8 0), !nosanitize [[META2]]
// TR-NEXT:    [[TMP7:%.*]] = xor i1 [[TMP6]], true, !nosanitize [[META2]]
// TR-NEXT:    [[TMP8:%.*]] = or i1 [[TMP5]], [[TMP7]], !nosanitize [[META2]]
// TR-NEXT:    br i1 [[TMP8]], label [[CONT:%.*]], label [[TRAP:%.*]], !nosanitize [[META2]]
// TR:       trap:
// TR-NEXT:    call void @llvm.ubsantrap(i8 0) #[[ATTR4]], !nosanitize [[META2]]
// TR-NEXT:    unreachable, !nosanitize [[META2]]
// TR:       cont:
// TR-NEXT:    ret i32 [[TMP3]]
//
// REC-LABEL: define dso_local i32 @overflow(
// REC-SAME: i32 noundef [[X:%.*]], i32 noundef [[Y:%.*]]) #[[ATTR0]] {
// REC-NEXT:  entry:
// REC-NEXT:    [[X_ADDR:%.*]] = alloca i32, align 4
// REC-NEXT:    [[Y_ADDR:%.*]] = alloca i32, align 4
// REC-NEXT:    store i32 [[X]], ptr [[X_ADDR]], align 4
// REC-NEXT:    store i32 [[Y]], ptr [[Y_ADDR]], align 4
// REC-NEXT:    [[TMP0:%.*]] = load i32, ptr [[X_ADDR]], align 4
// REC-NEXT:    [[TMP1:%.*]] = load i32, ptr [[Y_ADDR]], align 4
// REC-NEXT:    [[TMP2:%.*]] = call { i32, i1 } @llvm.sadd.with.overflow.i32(i32 [[TMP0]], i32 [[TMP1]]), !nosanitize [[META2]]
// REC-NEXT:    [[TMP3:%.*]] = extractvalue { i32, i1 } [[TMP2]], 0, !nosanitize [[META2]]
// REC-NEXT:    [[TMP4:%.*]] = extractvalue { i32, i1 } [[TMP2]], 1, !nosanitize [[META2]]
// REC-NEXT:    [[TMP5:%.*]] = xor i1 [[TMP4]], true, !nosanitize [[META2]]
// REC-NEXT:    [[TMP6:%.*]] = call i1 @llvm.allow.ubsan.check(i8 0), !nosanitize [[META2]]
// REC-NEXT:    [[TMP7:%.*]] = xor i1 [[TMP6]], true, !nosanitize [[META2]]
// REC-NEXT:    [[TMP8:%.*]] = or i1 [[TMP5]], [[TMP7]], !nosanitize [[META2]]
// REC-NEXT:    br i1 [[TMP8]], label [[CONT:%.*]], label [[HANDLER_ADD_OVERFLOW:%.*]], !prof [[PROF3]], !nosanitize [[META2]]
// REC:       handler.add_overflow:
// REC-NEXT:    [[TMP9:%.*]] = zext i32 [[TMP0]] to i64, !nosanitize [[META2]]
// REC-NEXT:    [[TMP10:%.*]] = zext i32 [[TMP1]] to i64, !nosanitize [[META2]]
// REC-NEXT:    call void @__ubsan_handle_add_overflow(ptr @[[GLOB3:[0-9]+]], i64 [[TMP9]], i64 [[TMP10]]) #[[ATTR4]], !nosanitize [[META2]]
// REC-NEXT:    br label [[CONT]], !nosanitize [[META2]]
// REC:       cont:
// REC-NEXT:    ret i32 [[TMP3]]
//
int overflow(int x, int y) {
  return x + y;
}
//.
// CHECK: [[META2]] = !{}
// CHECK: [[PROF3]] = !{!"branch_weights", i32 1048575, i32 1}
//.
// TR: [[META2]] = !{}
//.
// REC: [[META2]] = !{}
// REC: [[PROF3]] = !{!"branch_weights", i32 1048575, i32 1}
//.
