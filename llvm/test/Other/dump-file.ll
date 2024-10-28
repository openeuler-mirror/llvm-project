; Simple checks of -dump-file functionality
;
; Note that (mostly) only the banners are checked.
;
; Simple functionality check
; RUN: opt -S -passes=early-cse -print-after-all -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-AFTER-ALL
; 
; Simple functionality check with multiple passes
; RUN: opt -S -passes="function-attrs,early-cse" -print-after-all -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-AFTER-ALL-MULTI-PASSES
;
; Simple functionality check with -print-after-all and -filter-print-funcs
; RUN: opt -S -passes="function-attrs,early-cse" -print-after-all -filter-print-funcs=g -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-AFTER-ALL-FILTER-FUNC
;
; Simple functionality check with -print-after
; RUN: opt -S -passes="function-attrs,early-cse" -print-after=early-cse -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-AFTER
;
; Simple functionality check with multiple passes run on g are printed and the others are filtered out
; RUN: opt -S -passes="function-attrs,early-cse" -print-after=early-cse -filter-print-funcs=g -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-AFTER-FILTER-FUNC

; Simple functionality check with -print-changed 
; RUN: opt -S -O2 -print-changed -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-CHANGED
;
; Check that only the passes that change the IR are printed and that the
; others (including g) are filtered out.
; RUN: opt -S -print-changed -passes=early-cse -filter-print-funcs=f -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-CHANGED-FUNC-FILTER
;
; Check that reporting of multiple functions happens
; RUN: opt -S -print-changed -passes=early-cse -filter-print-funcs="f,g" -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-CHANGED-FILTER-MULT-FUNC
;
; Check that the reporting of IRs respects -filter-passes
; RUN: opt -S -print-changed -passes="function-attrs,early-cse" -filter-passes="function-attrs" -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-CHANGED-FILTER-PASSES
;
; Check that the reporting of IRs respects -filter-passes with multiple passes
; RUN: opt -S -print-changed -passes="function-attrs,early-cse" -filter-passes="function-attrs,early-cse" -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-CHANGED-FILTER-MULT-PASSES
;
; Check that the reporting of IRs respects both -filter-passes and -filter-print-funcs
; RUN: opt -S -print-changed -passes="function-attrs,early-cse" -filter-passes="function-attrs,early-cse" -filter-print-funcs=f -dump-file 2>&1 -o /dev/null < %s | FileCheck %s --check-prefix=CHECK-PRINT-CHANGED-FILTER-FUNC-PASSES

define i32 @g() {
entry:
  %a = add i32 2, 3
  ret i32 %a
}

define i32 @f() {
entry:
  %a = add i32 2, 3
  ret i32 %a
}


; CHECK-PRINT-AFTER-ALL: IR Dump After EarlyCSEPass on g to file at index - 1
; CHECK-PRINT-AFTER-ALL: IR Dump After EarlyCSEPass on f to file at index - 2

; CHECK-PRINT-AFTER-ALL-MULTI-PASSES: IR Dump After PostOrderFunctionAttrsPass on (g) to file at index - 1
; CHECK-PRINT-AFTER-ALL-MULTI-PASSES: IR Dump After EarlyCSEPass on g to file at index - 2
; CHECK-PRINT-AFTER-ALL-MULTI-PASSES: IR Dump After PostOrderFunctionAttrsPass on (f) to file at index - 3
; CHECK-PRINT-AFTER-ALL-MULTI-PASSES: IR Dump After EarlyCSEPass on f to file at index - 4

; CHECK-PRINT-AFTER-ALL-FILTER-FUNC: IR Dump After PostOrderFunctionAttrsPass on (g) to file at index - 1
; CHECK-PRINT-AFTER-ALL-FILTER-FUNC: IR Dump After EarlyCSEPass on g to file at index - 2

; CHECK-PRINT-AFTER: IR Dump After EarlyCSEPass on g to file at index - 2
; CHECK-PRINT-AFTER: IR Dump After EarlyCSEPass on f to file at index - 4

; CHECK-PRINT-AFTER-FILTER-FUNC: IR Dump After EarlyCSEPass on g to file at index - 2

; CHECK-PRINT-CHANGED: IR Dump At Start to file at index - 0
; CHECK-PRINT-CHANGED: IR Dump After EarlyCSEPass on g to file at index - 10
; CHECK-PRINT-CHANGED: IR Dump After EarlyCSEPass on f to file at index - 15
; CHECK-PRINT-CHANGED: IR Dump After GlobalOptPass on [module] to file at index - 19
; CHECK-PRINT-CHANGED: IR Dump After PostOrderFunctionAttrsPass on (g) to file at index - 87
; CHECK-PRINT-CHANGED: IR Dump After PostOrderFunctionAttrsPass on (f) to file at index - 142


; CHECK-PRINT-CHANGED-FUNC-FILTER: IR Dump At Start to file at index - 0
; CHECK-PRINT-CHANGED-FUNC-FILTER-NEXT: IR Dump After EarlyCSEPass on g filtered out
; CHECK-PRINT-CHANGED-FUNC-FILTER: IR Dump After EarlyCSEPass on f to file at index - 5

; CHECK-PRINT-CHANGED-FILTER-MULT-FUNC: IR Dump At Start to file at index - 0
; CHECK-PRINT-CHANGED-FILTER-MULT-FUNC-NEXT: IR Dump After EarlyCSEPass on g to file at index - 3
; CHECK-PRINT-CHANGED-FILTER-MULT-FUNC: IR Dump After EarlyCSEPass on f to file at index - 5

; CHECK-PRINT-CHANGED-FILTER-PASSES: IR Dump At Start to file at index - 0
; CHECK-PRINT-CHANGED-FILTER-PASSES: IR Dump After PostOrderFunctionAttrsPass on (g) to file at index - 3
; CHECK-PRINT-CHANGED-FILTER-PASSES-NEXT: IR Dump After EarlyCSEPass on g filtered out
; CHECK-PRINT-CHANGED-FILTER-PASSES: IR Dump After PostOrderFunctionAttrsPass on (f) to file at index - 7
; CHECK-PRINT-CHANGED-FILTER-PASSES-NEXT: IR Dump After EarlyCSEPass on f filtered out

; CHECK-PRINT-CHANGED-FILTER-MULT-PASSES: IR Dump At Start to file at index - 0
; CHECK-PRINT-CHANGED-FILTER-MULT-PASSES-NEXT: IR Dump After PostOrderFunctionAttrsPass on (g) to file at index - 3
; CHECK-PRINT-CHANGED-FILTER-MULT-PASSES: IR Dump After EarlyCSEPass on g to file at index - 5
; CHECK-PRINT-CHANGED-FILTER-MULT-PASSES: IR Dump After PostOrderFunctionAttrsPass on (f) to file at index - 7
; CHECK-PRINT-CHANGED-FILTER-MULT-PASSES-NEXT: IR Dump After EarlyCSEPass on f to file at index - 9
; CHECK-PRINT-CHANGED-FILTER-MULT-PASSES: IR Dump After VerifierPass on [module] filtered out

; CHECK-PRINT-CHANGED-FILTER-FUNC-PASSES: IR Dump At Start to file at index - 0
; CHECK-PRINT-CHANGED-FILTER-FUNC-PASSES: IR Dump After PostOrderFunctionAttrsPass on (g) omitted because no change
; CHECK-PRINT-CHANGED-FILTER-FUNC-PASSES: IR Dump After EarlyCSEPass on g filtered out
; CHECK-PRINT-CHANGED-FILTER-FUNC-PASSES: IR Dump After PostOrderFunctionAttrsPass on (f) to file at index - 7
; CHECK-PRINT-CHANGED-FILTER-FUNC-PASSES: IR Dump After EarlyCSEPass on f to file at index - 9
