; RUN: opt < %s -passes=instcombine -S | FileCheck %s
; In each test case, an extra instruction is introduced during the transformation,
; which will be eliminated in the subsequent dead code elimination optimization.

define double @pow_sqrt(double %x, double %y) {
; CHECK-LABEL: @pow_sqrt(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast double @sqrt(double [[X:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast double [[Y:%.*]], 5.000000e-01
; CHECK-NEXT:    [[POW:%.*]] = call fast double @pow(double [[X:%.*]], double [[MUL]])
; CHECK-NEXT:    ret double [[POW]]
;
  %call = call fast double @sqrt(double %x) 
  %pow = call fast double @pow(double %call, double %y)
  ret double %pow
}

define float @powf_sqrtf(float %x, float %y) {
; CHECK-LABEL: @powf_sqrtf(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast float @sqrtf(float [[X:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast float [[Y:%.*]], 5.000000e-01
; CHECK-NEXT:    [[POW:%.*]] = call fast float @powf(float [[X:%.*]], float [[MUL]])
; CHECK-NEXT:    ret float [[POW]]
;
  %call = call fast float @sqrtf(float %x) 
  %pow = call fast float @powf(float %call, float %y)
  ret float %pow
}

define double @pow_pow(double %x, double %y, double %z) {
; CHECK-LABEL: @pow_pow(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast double @pow(double [[X:%.*]], double [[Y:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast double [[Y:%.*]], [[Z:%.*]]
; CHECK-NEXT:    [[POW:%.*]] = call fast double @pow(double [[X:%.*]], double [[MUL]])
; CHECK-NEXT:    ret double [[POW]]
;
  %call = call fast double @pow(double %x, double %y) 
  %pow = call fast double @pow(double %call, double %z)
  ret double %pow
}

define float @powf_powf(float %x, float %y, float %z) {
; CHECK-LABEL: @powf_powf(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast float @powf(float [[X:%.*]], float [[Y:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast float [[Y:%.*]], [[Z:%.*]]
; CHECK-NEXT:    [[POW:%.*]] = call fast float @powf(float [[X:%.*]], float [[MUL]])
; CHECK-NEXT:    ret float [[POW]]
;
  %call = call fast float @powf(float %x, float %y) 
  %pow = call fast float @powf(float %call, float %z)
  ret float %pow
}

define double @sqrt_nroot(double %x, double %n){
; CHECK-LABEL: @sqrt_nroot(
; CHECK-NEXT:    [[DIV:%.*]] = fdiv double 1.000000e+00, [[N:%.*]]
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast double @pow(double [[X:%.*]], double [[DIV]])
; CHECK-NEXT:    [[ABSX:%.*]] = call fast double @llvm.fabs.f64(double [[X:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast double [[DIV]], 5.000000e-01
; CHECK-NEXT:    [[POW:%.*]] = call fast double @pow(double [[ABSX]], double [[MUL]])
; CHECK-NEXT:    ret double [[POW]]
;
  %div = fdiv double 1.000000e+00, %n
  %call = call fast double @pow(double %x, double %div)
  %call6 = call fast double @sqrt(double %call)
  ret double %call6
}

define float @sqrtf_nroot(float %x, float %n){
; CHECK-LABEL: @sqrtf_nroot(
; CHECK-NEXT:    [[DIV:%.*]] = fdiv float 1.000000e+00, [[N:%.*]]
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast float @powf(float [[X:%.*]], float [[DIV]])
; CHECK-NEXT:    [[ABSX:%.*]] = call fast float @llvm.fabs.f32(float [[X:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast float [[DIV]], 5.000000e-01
; CHECK-NEXT:    [[POW:%.*]] = call fast float @powf(float [[ABSX]], float [[MUL]])
; CHECK-NEXT:    ret float [[POW]]
;
  %div = fdiv float 1.000000e+00, %n
  %call = call fast float @powf(float %x, float %div)
  %call6 = call fast float @sqrtf(float %call)
  ret float %call6
}

define double @sqrt_pow(double %x, double %y) {
; CHECK-LABEL: @sqrt_pow(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast double  @pow(double [[X:%.*]], double [[Y:%.*]])
; CHECK-NEXT:    [[ABSX:%.*]] = call fast double @llvm.fabs.f64(double [[X:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast double [[Y:%.*]], 5.000000e-01
; CHECK-NEXT:    [[POW:%.*]] = call fast double @pow(double [[ABSX]], double [[MUL]])
; CHECK-NEXT:    ret double [[POW]]
;
  %call = call fast double @pow(double %x, double %y) 
  %pow = call fast double @sqrt(double %call)
  ret double %pow
}

define float @sqrtf_powf(float %x, float %y) {
; CHECK-LABEL: @sqrtf_powf(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast float  @powf(float [[X:%.*]], float [[Y:%.*]])
; CHECK-NEXT:    [[ABSX:%.*]] = call fast float @llvm.fabs.f32(float [[X:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast float [[Y:%.*]], 5.000000e-01
; CHECK-NEXT:    [[POW:%.*]] = call fast float @powf(float [[ABSX]], float [[MUL]])
; CHECK-NEXT:    ret float [[POW]]
;
  %call = call fast float @powf(float %x, float %y) 
  %pow = call fast float @sqrtf(float %call)
  ret float %pow
}

define double @cbrt_exp(double %x) {
; CHECK-LABEL: @cbrt_exp(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast double  @exp(double [[X:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast double [[X:%.*]], 0x3FD5555555555555
; CHECK-NEXT:    [[EXP:%.*]] = call fast double @exp(double [[MUL]])
; CHECK-NEXT:    ret double [[EXP]]
;
  %call = call fast double @exp(double %x) 
  %pow = call fast double @cbrt(double %call)
  ret double %pow
}

define float @cbrtf_expf(float %x) {
; CHECK-LABEL: @cbrtf_expf(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast float  @expf(float [[X:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast float [[X:%.*]], 0x3FD5555560000000
; CHECK-NEXT:    [[EXP:%.*]] = call fast float @expf(float [[MUL]])
; CHECK-NEXT:    ret float [[EXP]]
;
  %call = call fast float @expf(float %x) 
  %pow = call fast float @cbrtf(float %call)
  ret float %pow
}

define double @cbrt_exp2(double %x) {
; CHECK-LABEL: @cbrt_exp2(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast double  @exp2(double [[X:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast double [[X:%.*]], 0x3FD5555555555555
; CHECK-NEXT:    [[EXP:%.*]] = call fast double @exp2(double [[MUL]])
; CHECK-NEXT:    ret double [[EXP]]
;
  %call = call fast double @exp2(double %x) 
  %pow = call fast double @cbrt(double %call)
  ret double %pow
}

define float @cbrtf_exp2f(float %x) {
; CHECK-LABEL: @cbrtf_exp2f(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast float  @exp2f(float [[X:%.*]])
; CHECK-NEXT:    [[MUL:%.*]] = fmul fast float [[X:%.*]], 0x3FD5555560000000
; CHECK-NEXT:    [[EXP:%.*]] = call fast float @exp2f(float [[MUL]])
; CHECK-NEXT:    ret float [[EXP]]
;
  %call = call fast float @exp2f(float %x) 
  %pow = call fast float @cbrtf(float %call)
  ret float %pow
}

define double @cbrt_sqrt(double %x) {
; CHECK-LABEL: @cbrt_sqrt(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast double  @sqrt(double [[X:%.*]])
; CHECK-NEXT:    [[POW:%.*]] = call fast double @pow(double [[X:%.*]], double 0x3FC5555555555555)
; CHECK-NEXT:    ret double [[POW]]
;
  %call = call fast double @sqrt(double %x) 
  %pow = call fast double @cbrt(double %call)
  ret double %pow
}

define float @cbrtf_sqrtf(float %x) {
; CHECK-LABEL: @cbrtf_sqrtf(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast float  @sqrtf(float [[X:%.*]])
; CHECK-NEXT:    [[POW:%.*]] = call fast float @powf(float [[X:%.*]], float 0x3FC5555560000000)
; CHECK-NEXT:    ret float [[POW]]
;
  %call = call fast float @sqrtf(float %x) 
  %pow = call fast float @cbrtf(float %call)
  ret float %pow
}

define double @cbrt_cbrt(double %x) {
; CHECK-LABEL: @cbrt_cbrt(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast double  @cbrt(double [[X:%.*]])
; CHECK-NEXT:    [[POW1:%.*]] = call fast double @pow(double [[X:%.*]], double 0x3FBC71C71C71C71C)
; CHECK-NEXT:    [[NEG_X:%.*]] = fneg fast double [[X:%.*]]
; CHECK-NEXT:    [[POW2:%.*]] = call fast double @pow(double [[NEG_X]], double 0x3FBC71C71C71C71C)
; CHECK-NEXT:    [[NEG_POW2:%.*]] = fneg fast double [[POW2]]
; CHECK-NEXT:    [[CMP:%.*]] = fcmp fast oge double [[X:%.*]], 0.000000e+00
; CHECK-NEXT:    [[SELECT:%.*]] = select fast i1 [[CMP]], double [[POW1]], double [[NEG_POW2]]
; CHECK-NEXT:    ret double [[SELECT]]
;
  %call = call fast double @cbrt(double %x) 
  %pow = call fast double @cbrt(double %call)
  ret double %pow
}

define float @cbrtf_cbrtf(float %x) {
; CHECK-LABEL: @cbrtf_cbrtf(
; CHECK-NEXT:    [[UNUSED:%.*]] = call fast float  @cbrtf(float [[X:%.*]])
; CHECK-NEXT:    [[POW1:%.*]] = call fast float @powf(float [[X:%.*]], float 0x3FBC71C720000000)
; CHECK-NEXT:    [[NEG_X:%.*]] = fneg fast float [[X:%.*]]
; CHECK-NEXT:    [[POW2:%.*]] = call fast float @powf(float [[NEG_X]], float 0x3FBC71C720000000)
; CHECK-NEXT:    [[NEG_POW2:%.*]] = fneg fast float [[POW2]]
; CHECK-NEXT:    [[CMP:%.*]] = fcmp fast oge float [[X:%.*]], 0.000000e+00
; CHECK-NEXT:    [[SELECT:%.*]] = select fast i1 [[CMP]], float [[POW1]], float [[NEG_POW2]]
; CHECK-NEXT:    ret float [[SELECT]]
;
  %call = call fast float @cbrtf(float %x) 
  %pow = call fast float @cbrtf(float %call)
  ret float %pow
}

declare double @pow(double,double)
declare float @powf(float,float)
declare double @sqrt(double)
declare float @sqrtf(float)
declare double @cbrt(double)
declare float @cbrtf(float)
declare double @exp(double)
declare float @expf(float)
declare double @exp2(double)
declare float @exp2f(float)
declare double @llvm.fabs.f64(double)
declare float @llvm.fabs.f32(float)
