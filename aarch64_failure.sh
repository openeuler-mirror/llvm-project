#!/bin/bash

# 过滤llvm-libc++失败用例
export LIT_FILTER_OUT="llvm-libc\+\+-shared\.cfg\.in.*(leap_seconds\.pass\.cpp|get_tzdb\.pass\.cpp|date\.pass\.cpp|value\.pass\.cpp|comparison\.pass\.cpp|sys_info\.zdump\.pass\.cpp)"
# 过滤llvm-libc++失败用例 (dev_18.1.8)
export LIT_FILTER_OUT="llvm-libc\+\+-shared\.cfg\.in.*(get_leap_second_info\.pass\.cpp|from_sys\.pass\.cpp|to_sys\.pass\.cpp)|${LIT_FILTER_OUT}"
# 过滤llvm-libc++失败用例 (trunk)
export LIT_FILTER_OUT="llvm-libc\+\+-shared\.cfg\.in.*(from_utc\.pass\.cpp|to_utc\.pass\.cpp)|${LIT_FILTER_OUT}"
# 过滤随机失败用例（本地验证用例通过）
#   LeakSanitizer-Standalone-aarch64 :: TestCases/use_registers.cpp
export LIT_FILTER_OUT="TestCases\/use_registers\.cpp|${LIT_FILTER_OUT}"
# 过滤已知开源失败用例
#   libomptarget :: aarch64-unknown-linux-gnu :: mapping/target_derefence_array_pointrs.cpp
#   libomptarget :: aarch64-unknown-linux-gnu-LTO :: mapping/target_derefence_array_pointrs.cpp
export LIT_FILTER_OUT="mapping\/target_derefence_array_pointrs\.cpp|${LIT_FILTER_OUT}"
# 过滤在线CI失败用例（本地验证用例通过）
#   libomptarget :: aarch64-unknown-linux-gnu-LTO :: offloading/target_nowait_target.cpp
export LIT_FILTER_OUT="offloading\/target_nowait_target\.cpp|${LIT_FILTER_OUT}"
# 过滤在线CI超时bolt用例
#   BOLT :: runtime/instrumentation-indirect-2.c
export LIT_FILTER_OUT="runtime\/instrumentation-indirect-2.c|${LIT_FILTER_OUT}"

if [ "$tbranch" = "dev_19.1.7" ]; then
# 过滤dev_19.1.7分支失败用例
# 过滤在线CI失败用例（本地验证用例通过）
#   libomp :: affinity/kmp-abs-hw-subset.c
export LIT_FILTER_OUT="affinity\/kmp-abs-hw-subset\.c|${LIT_FILTER_OUT}"
#   Profile-aarch64 :: instrprof-basic.c
#   https://gitee.com/openeuler/llvm-project/issues/ID2EVL?from=project-issue
export LIT_FILTER_OUT="instrprof-basic\.c|${LIT_FILTER_OUT}"
fi

if [ "$tbranch" = "dev_18.1.8" ]; then
# 过滤dev_18.1.8分支失败用例
# 过滤在线CI失败用例（本地验证用例通过）
#   libarcher :: races/task-taskgroup-unrelated.c
#   https://gitcode.com/openeuler/llvm-project/issues/102
export LIT_FILTER_OUT="races\/task-taskgroup-unrelated\.c|${LIT_FILTER_OUT}"
fi

if [[ "$tbranch" = "dev_16.0.6" ||
      "$tbranch" = "feature-thinlto-split" ||
      "$tbranch" = "feature-hip12-optimization-dev16" ]]; then
# 过滤dev_16.0.6分支失败用例
#   libomptarget :: aarch64-unknown-linux-gnu :: mapping/delete_inf_refcount.c
#   libomptarget :: aarch64-unknown-linux-gnu :: mapping/ompx_hold/struct.c
#   libomptarget :: aarch64-unknown-linux-gnu :: offloading/global_constructor.cpp
#   libomptarget :: aarch64-unknown-linux-gnu :: offloading/static_linking.c
#   libomptarget :: aarch64-unknown-linux-gnu-LTO :: mapping/delete_inf_refcount.c
#   libomptarget :: aarch64-unknown-linux-gnu-LTO :: mapping/ompx_hold/struct.c
#   libomptarget :: aarch64-unknown-linux-gnu-LTO :: offloading/global_constructor.cpp
#   libomptarget :: aarch64-unknown-linux-gnu-LTO :: offloading/static_linking.c
export LIT_FILTER_OUT="mapping\/delete_inf_refcount\.c|${LIT_FILTER_OUT}"
export LIT_FILTER_OUT="mapping\/ompx_hold\/struct\.c|${LIT_FILTER_OUT}"
export LIT_FILTER_OUT="offloading\/global_constructor\.cpp|${LIT_FILTER_OUT}"
export LIT_FILTER_OUT="offloading\/static_linking\.c|${LIT_FILTER_OUT}"
fi
