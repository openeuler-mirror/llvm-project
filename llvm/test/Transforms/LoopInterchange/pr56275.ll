; NOTE: Assertions have been autogenerated by utils/update_test_checks.py
; RUN: opt < %s -aa-pipeline=basic-aa -passes=loop-interchange -cache-line-size=64 -verify-dom-info -verify-loop-info -verify-scev -verify-loop-lcssa -S | FileCheck %s

target datalayout = "e-m:e-i8:8:32-i16:16:32-i64:64-i128:128-n32:64-S128"
target triple = "aarch64-unknown-linux-gnu"

;; Test to make sure DA outputs the correction direction
;; vector [< =] hence the loopnest is interchanged.
;;
;; void test1(unsigned a[restrict N1][N2],
;;          unsigned b[restrict N1][N2],
;;          unsigned c[restrict N1][N2]) {
;;  for (unsigned long i2 = 1; i2 < N2-1; i2++) {
;;    for (unsigned long i1 = 1; i1 < N1-1; i1++) {
;;      a[i1][i2+1] = b[i1][i2];
;;      c[i1][i2] = a[i1][i2];
;;    }
;;  }
;; }

define void @test1(ptr noalias noundef %a, ptr noalias noundef %b, ptr noalias noundef %c) {
; CHECK-LABEL: @test1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label [[LOOP2_HEADER_PREHEADER:%.*]]
; CHECK:       loop1.header.preheader:
; CHECK-NEXT:    br label [[LOOP1_HEADER:%.*]]
; CHECK:       loop1.header:
; CHECK-NEXT:    [[I2:%.*]] = phi i64 [ [[I2_INC:%.*]], [[LOOP1_LATCH:%.*]] ], [ 1, [[LOOP1_HEADER_PREHEADER:%.*]] ]
; CHECK-NEXT:    [[I2_ST:%.*]] = add i64 [[I2]], 1
; CHECK-NEXT:    [[I2_LD:%.*]] = add i64 [[I2]], 0
; CHECK-NEXT:    br label [[LOOP2_HEADER_SPLIT1:%.*]]
; CHECK:       loop2.header.preheader:
; CHECK-NEXT:    br label [[LOOP2_HEADER:%.*]]
; CHECK:       loop2.header:
; CHECK-NEXT:    [[I1:%.*]] = phi i64 [ [[TMP0:%.*]], [[LOOP2_HEADER_SPLIT:%.*]] ], [ 1, [[LOOP2_HEADER_PREHEADER]] ]
; CHECK-NEXT:    br label [[LOOP1_HEADER_PREHEADER]]
; CHECK:       loop2.header.split1:
; CHECK-NEXT:    [[I1_ST:%.*]] = add i64 [[I1]], 0
; CHECK-NEXT:    [[I1_LD:%.*]] = add i64 [[I1]], 0
; CHECK-NEXT:    [[A_ST:%.*]] = getelementptr inbounds [64 x i32], ptr [[A:%.*]], i64 [[I1_ST]], i64 [[I2_ST]]
; CHECK-NEXT:    [[A_LD:%.*]] = getelementptr inbounds [64 x i32], ptr [[A]], i64 [[I1_LD]], i64 [[I2_LD]]
; CHECK-NEXT:    [[B_LD:%.*]] = getelementptr inbounds [64 x i32], ptr [[B:%.*]], i64 [[I1]], i64 [[I2]]
; CHECK-NEXT:    [[C_ST:%.*]] = getelementptr inbounds [64 x i32], ptr [[C:%.*]], i64 [[I1]], i64 [[I2]]
; CHECK-NEXT:    [[B_VAL:%.*]] = load i32, ptr [[B_LD]], align 4
; CHECK-NEXT:    store i32 [[B_VAL]], ptr [[A_ST]], align 4
; CHECK-NEXT:    [[A_VAL:%.*]] = load i32, ptr [[A_LD]], align 4
; CHECK-NEXT:    store i32 [[A_VAL]], ptr [[C_ST]], align 4
; CHECK-NEXT:    [[I1_INC:%.*]] = add nuw nsw i64 [[I1]], 1
; CHECK-NEXT:    [[LOOP2_EXITCOND_NOT:%.*]] = icmp eq i64 [[I1_INC]], 63
; CHECK-NEXT:    br label [[LOOP1_LATCH]]
; CHECK:       loop2.header.split:
; CHECK-NEXT:    [[TMP0]] = add nuw nsw i64 [[I1]], 1
; CHECK-NEXT:    [[TMP1:%.*]] = icmp eq i64 [[TMP0]], 63
; CHECK-NEXT:    br i1 [[TMP1]], label [[EXIT:%.*]], label [[LOOP2_HEADER]]
; CHECK:       loop1.latch:
; CHECK-NEXT:    [[I2_INC]] = add nuw nsw i64 [[I2]], 1
; CHECK-NEXT:    [[LOOP1_EXITCOND_NOT:%.*]] = icmp eq i64 [[I2_INC]], 63
; CHECK-NEXT:    br i1 [[LOOP1_EXITCOND_NOT]], label [[LOOP2_HEADER_SPLIT]], label [[LOOP1_HEADER]]
; CHECK:       exit:
; CHECK-NEXT:    ret void
;
entry:
  br label %loop1.header

loop1.header:
  %i2 = phi i64 [ 1, %entry ], [ %i2.inc, %loop1.latch ]
  %i2.st = add i64 %i2, 1
  %i2.ld = add i64 %i2, 0
  br label %loop2.header

loop2.header:
  %i1 = phi i64 [ 1, %loop1.header ], [ %i1.inc, %loop2.header ]
  %i1.st = add i64 %i1, 0
  %i1.ld = add i64 %i1, 0
  %a.st = getelementptr inbounds [64 x i32], ptr %a, i64 %i1.st, i64 %i2.st
  %a.ld = getelementptr inbounds [64 x i32], ptr %a, i64 %i1.ld, i64 %i2.ld
  %b.ld = getelementptr inbounds [64 x i32], ptr %b, i64 %i1, i64 %i2
  %c.st = getelementptr inbounds [64 x i32], ptr %c, i64 %i1, i64 %i2
  %b.val = load i32, ptr %b.ld, align 4
  store i32 %b.val, ptr %a.st, align 4  ; (X) store to a[i1][i2+1]
  %a.val = load i32, ptr %a.ld, align 4 ; (Y) load from a[i1][i2]
  store i32 %a.val, ptr %c.st, align 4
  %i1.inc = add nuw nsw i64 %i1, 1
  %loop2.exitcond.not = icmp eq i64 %i1.inc, 63
  br i1 %loop2.exitcond.not, label %loop1.latch, label %loop2.header

loop1.latch:
  %i2.inc = add nuw nsw i64 %i2, 1
  %loop1.exitcond.not = icmp eq i64 %i2.inc, 63
  br i1 %loop1.exitcond.not, label %exit, label %loop1.header

exit:
  ret void
}

;; Semantically equivalent to test1() with only the difference
;; of the order of a load and a store at (X) and (Y).
;;
;; Test to make sure DA outputs the correction direction
;; vector [< =] hence the loopnest is interchanged.

define void @test2(ptr noalias noundef %a, ptr noalias noundef %b, ptr noalias noundef %c) {
; CHECK-LABEL: @test2(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    br label [[LOOP2_HEADER_PREHEADER:%.*]]
; CHECK:       loop1.header.preheader:
; CHECK-NEXT:    br label [[LOOP1_HEADER:%.*]]
; CHECK:       loop1.header:
; CHECK-NEXT:    [[I2:%.*]] = phi i64 [ [[I2_INC:%.*]], [[LOOP1_LATCH:%.*]] ], [ 1, [[LOOP1_HEADER_PREHEADER:%.*]] ]
; CHECK-NEXT:    [[I2_ST:%.*]] = add i64 [[I2]], 1
; CHECK-NEXT:    [[I2_LD:%.*]] = add i64 [[I2]], 0
; CHECK-NEXT:    br label [[LOOP2_HEADER_SPLIT1:%.*]]
; CHECK:       loop2.header.preheader:
; CHECK-NEXT:    br label [[LOOP2_HEADER:%.*]]
; CHECK:       loop2.header:
; CHECK-NEXT:    [[I1:%.*]] = phi i64 [ [[TMP0:%.*]], [[LOOP2_HEADER_SPLIT:%.*]] ], [ 1, [[LOOP2_HEADER_PREHEADER]] ]
; CHECK-NEXT:    br label [[LOOP1_HEADER_PREHEADER]]
; CHECK:       loop2.header.split1:
; CHECK-NEXT:    [[I1_ST:%.*]] = add i64 [[I1]], 0
; CHECK-NEXT:    [[I1_LD:%.*]] = add i64 [[I1]], 0
; CHECK-NEXT:    [[A_ST:%.*]] = getelementptr inbounds [64 x i32], ptr [[A:%.*]], i64 [[I1_ST]], i64 [[I2_ST]]
; CHECK-NEXT:    [[A_LD:%.*]] = getelementptr inbounds [64 x i32], ptr [[A]], i64 [[I1_LD]], i64 [[I2_LD]]
; CHECK-NEXT:    [[B_LD:%.*]] = getelementptr inbounds [64 x i32], ptr [[B:%.*]], i64 [[I1]], i64 [[I2]]
; CHECK-NEXT:    [[C_ST:%.*]] = getelementptr inbounds [64 x i32], ptr [[C:%.*]], i64 [[I1]], i64 [[I2]]
; CHECK-NEXT:    [[B_VAL:%.*]] = load i32, ptr [[B_LD]], align 4
; CHECK-NEXT:    [[A_VAL:%.*]] = load i32, ptr [[A_LD]], align 4
; CHECK-NEXT:    store i32 [[B_VAL]], ptr [[A_ST]], align 4
; CHECK-NEXT:    store i32 [[A_VAL]], ptr [[C_ST]], align 4
; CHECK-NEXT:    [[I1_INC:%.*]] = add nuw nsw i64 [[I1]], 1
; CHECK-NEXT:    [[LOOP2_EXITCOND_NOT:%.*]] = icmp eq i64 [[I1_INC]], 63
; CHECK-NEXT:    br label [[LOOP1_LATCH]]
; CHECK:       loop2.header.split:
; CHECK-NEXT:    [[TMP0]] = add nuw nsw i64 [[I1]], 1
; CHECK-NEXT:    [[TMP1:%.*]] = icmp eq i64 [[TMP0]], 63
; CHECK-NEXT:    br i1 [[TMP1]], label [[EXIT:%.*]], label [[LOOP2_HEADER]]
; CHECK:       loop1.latch:
; CHECK-NEXT:    [[I2_INC]] = add nuw nsw i64 [[I2]], 1
; CHECK-NEXT:    [[LOOP1_EXITCOND_NOT:%.*]] = icmp eq i64 [[I2_INC]], 63
; CHECK-NEXT:    br i1 [[LOOP1_EXITCOND_NOT]], label [[LOOP2_HEADER_SPLIT]], label [[LOOP1_HEADER]]
; CHECK:       exit:
; CHECK-NEXT:    ret void
;
entry:
  br label %loop1.header

loop1.header:
  %i2 = phi i64 [ 1, %entry ], [ %i2.inc, %loop1.latch ]
  %i2.st = add i64 %i2, 1
  %i2.ld = add i64 %i2, 0
  br label %loop2.header

loop2.header:
  %i1 = phi i64 [ 1, %loop1.header ], [ %i1.inc, %loop2.header ]
  %i1.st = add i64 %i1, 0
  %i1.ld = add i64 %i1, 0
  %a.st = getelementptr inbounds [64 x i32], ptr %a, i64 %i1.st, i64 %i2.st
  %a.ld = getelementptr inbounds [64 x i32], ptr %a, i64 %i1.ld, i64 %i2.ld
  %b.ld = getelementptr inbounds [64 x i32], ptr %b, i64 %i1, i64 %i2
  %c.st = getelementptr inbounds [64 x i32], ptr %c, i64 %i1, i64 %i2
  %b.val = load i32, ptr %b.ld, align 4
  %a.val = load i32, ptr %a.ld, align 4 ; (Y) load from a[i1][i2]
  store i32 %b.val, ptr %a.st, align 4  ; (X) store to a[i1][i2+1]
  store i32 %a.val, ptr %c.st, align 4
  %i1.inc = add nuw nsw i64 %i1, 1
  %loop2.exitcond.not = icmp eq i64 %i1.inc, 63
  br i1 %loop2.exitcond.not, label %loop1.latch, label %loop2.header

loop1.latch:
  %i2.inc = add nuw nsw i64 %i2, 1
  %loop1.exitcond.not = icmp eq i64 %i2.inc, 63
  br i1 %loop1.exitcond.not, label %exit, label %loop1.header

exit:
  ret void
}
