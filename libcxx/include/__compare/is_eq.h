//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___COMPARE_IS_EQ_H
#define _LIBCUDACXX___COMPARE_IS_EQ_H

#include <__compare/ordering.h>
#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_HIDE_FROM_ABI inline constexpr bool is_eq(partial_ordering __c) noexcept { return __c == 0; }
_LIBCUDACXX_HIDE_FROM_ABI inline constexpr bool is_neq(partial_ordering __c) noexcept { return __c != 0; }
_LIBCUDACXX_HIDE_FROM_ABI inline constexpr bool is_lt(partial_ordering __c) noexcept { return __c < 0; }
_LIBCUDACXX_HIDE_FROM_ABI inline constexpr bool is_lteq(partial_ordering __c) noexcept { return __c <= 0; }
_LIBCUDACXX_HIDE_FROM_ABI inline constexpr bool is_gt(partial_ordering __c) noexcept { return __c > 0; }
_LIBCUDACXX_HIDE_FROM_ABI inline constexpr bool is_gteq(partial_ordering __c) noexcept { return __c >= 0; }

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___COMPARE_IS_EQ_H
