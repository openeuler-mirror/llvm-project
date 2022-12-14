//===-- Definition of signal number macros --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef __LLVM_LIBC_MACROS_SIGNUM_MACROS_H
#define __LLVM_LIBC_MACROS_SIGNUM_MACROS_H

#ifdef __linux__
#include "linux/signal-macros.h"
#endif

#endif // __LLVM_LIBC_MACROS_SIGNUM_MACROS_H
