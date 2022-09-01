// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___UTILITY_AUTO_CAST_H
#define _LIBCUDACXX___UTILITY_AUTO_CAST_H

#include <__config>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#define _LIBCUDACXX_AUTO_CAST(expr) static_cast<typename decay<decltype((expr))>::type>(expr)

#endif // _LIBCUDACXX___UTILITY_AUTO_CAST_H
