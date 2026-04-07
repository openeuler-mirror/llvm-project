#!/bin/bash

# 过滤llvm-libc++失败用例
export LIT_FILTER_OUT="llvm-libc\+\+-shared\.cfg\.in.*(leap_seconds\.pass\.cpp|get_tzdb\.pass\.cpp|date\.pass\.cpp|value\.pass\.cpp|comparison\.pass\.cpp|sys_info\.zdump\.pass\.cpp)"
# 过滤llvm-libc++失败用例 (dev_18.1.8)
export LIT_FILTER_OUT="llvm-libc\+\+-shared\.cfg\.in.*(get_leap_second_info\.pass\.cpp|from_sys\.pass\.cpp|to_sys\.pass\.cpp)|${LIT_FILTER_OUT}"
# 过滤llvm-libc++失败用例 (trunk)
export LIT_FILTER_OUT="llvm-libc\+\+-shared\.cfg\.in.*(from_utc\.pass\.cpp|to_utc\.pass\.cpp)|${LIT_FILTER_OUT}"
# 过滤概率超时导致失败用例
# Timed Out Tests :
#   Clang :: Driver/emit-reproducer.c
#   Clang :: Driver/crash-report.cpp
export LIT_FILTER_OUT="Driver\/emit-reproducer\.c|Driver\/crash-report\.cpp|${LIT_FILTER_OUT}"
# 过滤在线CI卡住bolt用例
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

if [[ "$tbranch" = "dev_16.0.6" || "$tbranch" = "feature-thinlto-split" ]]; then
# 过滤dev_16.0.6分支失败用例
#   libomptarget :: x86_64-pc-linux-gnu :: mapping/delete_inf_refcount.c
#   libomptarget :: x86_64-pc-linux-gnu :: offloading/global_constructor.cpp
#   libomptarget :: x86_64-pc-linux-gnu :: offloading/static_linking.c
#   libomptarget :: x86_64-pc-linux-gnu-LTO :: mapping/delete_inf_refcount.c
#   libomptarget :: x86_64-pc-linux-gnu-LTO :: offloading/global_constructor.cpp
#   libomptarget :: x86_64-pc-linux-gnu-LTO :: offloading/static_linking.c
export LIT_FILTER_OUT="mapping\/delete_inf_refcount\.c|${LIT_FILTER_OUT}"
export LIT_FILTER_OUT="offloading\/global_constructor\.cpp|${LIT_FILTER_OUT}"
export LIT_FILTER_OUT="offloading\/static_linking\.c|${LIT_FILTER_OUT}"
fi
