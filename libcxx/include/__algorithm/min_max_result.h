// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_MIN_MAX_RESULT_H
#define _LIBCUDACXX___ALGORITHM_MIN_MAX_RESULT_H

#include <__concepts/convertible_to.h>
#include <__config>
#include <__utility/move.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_PUSH_MACROS
#include <__undef_macros>

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17 && !defined(_LIBCUDACXX_HAS_NO_INCOMPLETE_RANGES)

namespace ranges {

template <class _T1>
struct min_max_result {
  _LIBCUDACXX_NO_UNIQUE_ADDRESS _T1 min;
  _LIBCUDACXX_NO_UNIQUE_ADDRESS _T1 max;

  template <class _T2>
    requires convertible_to<const _T1&, _T2>
  _LIBCUDACXX_HIDE_FROM_ABI constexpr operator min_max_result<_T2>() const & {
    return {min, max};
  }

  template <class _T2>
    requires convertible_to<_T1, _T2>
  _LIBCUDACXX_HIDE_FROM_ABI constexpr operator min_max_result<_T2>() && {
    return {std::move(min), std::move(max)};
  }
};

} // namespace ranges

#endif // _LIBCUDACXX_STD_VER > 17 && !defined(_LIBCUDACXX_HAS_NO_INCOMPLETE_RANGES)

_LIBCUDACXX_END_NAMESPACE_STD

_LIBCUDACXX_POP_MACROS

#endif // _LIBCUDACXX___ALGORITHM_MIN_MAX_RESULT_H
