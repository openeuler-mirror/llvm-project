//===- WeakConsistencyConfig.h - Weak Consistency Pass ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
 
#ifndef LLVM_LIB_TARGET_AARCH64_WEAKCONSISTENCYCONFIG_H
#define LLVM_LIB_TARGET_AARCH64_WEAKCONSISTENCYCONFIG_H
 
#include "llvm/Support/WithColor.h":
#include "Utils/AArch64BaseInfo.h"
#include "AArch64RegisterInfo.h"
 
enum RELAXED_ORDERING_LEVEL : char {
    RO_DISABLE = 0,
    RO_1 = 1,
    RO_2 = 2,
    RO_3 = 3,
};
 
#endif

