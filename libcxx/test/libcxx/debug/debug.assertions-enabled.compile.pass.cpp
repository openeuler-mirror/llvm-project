//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// This test ensures that assertions are enabled by default when the debug mode is enabled.

// REQUIRES: LIBCUDACXX-has-debug-mode

#include <version>

#if !defined(_LIBCUDACXX_ENABLE_ASSERTIONS) || _LIBCUDACXX_ENABLE_ASSERTIONS == 0
#   error "Assertions should be enabled automatically when the debug mode is enabled"
#endif
