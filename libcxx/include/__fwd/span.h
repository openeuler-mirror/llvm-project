// -*- C++ -*-
//===---------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#ifndef _LIBCUDACXX_FWD_SPAN_H
#define _LIBCUDACXX_FWD_SPAN_H

#include <__config>
#include <cstddef>
#include <limits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_PUSH_MACROS
#include <__undef_macros>

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

inline constexpr size_t dynamic_extent = numeric_limits<size_t>::max();
template <typename _Tp, size_t _Extent = dynamic_extent> class span;

#endif

_LIBCUDACXX_END_NAMESPACE_STD

_LIBCUDACXX_POP_MACROS

#endif // _LIBCUDACXX_FWD_SPAN_H
