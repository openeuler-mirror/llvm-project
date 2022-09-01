// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_IN_FOUND_RESULT_H
#define _LIBCUDACXX___ALGORITHM_IN_FOUND_RESULT_H

#include <__concepts/convertible_to.h>
#include <__config>
#include <__utility/move.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCUDACXX_STD_VER > 17 && !defined(_LIBCUDACXX_HAS_NO_INCOMPLETE_RANGES)

_LIBCUDACXX_BEGIN_NAMESPACE_STD

namespace ranges {
template <class _InIter1>
struct in_found_result {
  _LIBCUDACXX_NO_UNIQUE_ADDRESS _InIter1 in;
  bool found;

  template <class _InIter2>
    requires convertible_to<const _InIter1&, _InIter2>
  _LIBCUDACXX_HIDE_FROM_ABI constexpr operator in_found_result<_InIter2>() const & {
    return {in, found};
  }

  template <class _InIter2>
    requires convertible_to<_InIter1, _InIter2>
  _LIBCUDACXX_HIDE_FROM_ABI constexpr operator in_found_result<_InIter2>() && {
    return {std::move(in), found};
  }
};
} // namespace ranges

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX_STD_VER > 17 && !defined(_LIBCUDACXX_HAS_NO_INCOMPLETE_RANGES)

#endif // _LIBCUDACXX___ALGORITHM_IN_FOUND_RESULT_H
