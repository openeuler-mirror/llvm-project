// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_ADJACENT_FIND_H
#define _LIBCUDACXX___ALGORITHM_ADJACENT_FIND_H

#include <__algorithm/comp.h>
#include <__algorithm/iterator_operations.h>
#include <__config>
#include <__iterator/iterator_traits.h>
#include <__utility/move.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Iter, class _Sent, class _BinaryPredicate>
_LIBCUDACXX_NODISCARD_EXT _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 _Iter
__adjacent_find(_Iter __first, _Sent __last, _BinaryPredicate&& __pred) {
  if (__first == __last)
    return __first;
  _Iter __i = __first;
  while (++__i != __last) {
    if (__pred(*__first, *__i))
      return __first;
    __first = __i;
  }
  return __i;
}

template <class _ForwardIterator, class _BinaryPredicate>
_LIBCUDACXX_NODISCARD_EXT inline _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 _ForwardIterator
adjacent_find(_ForwardIterator __first, _ForwardIterator __last, _BinaryPredicate __pred) {
  return std::__adjacent_find(std::move(__first), std::move(__last), __pred);
}

template <class _ForwardIterator>
_LIBCUDACXX_NODISCARD_EXT inline _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 _ForwardIterator
adjacent_find(_ForwardIterator __first, _ForwardIterator __last) {
  typedef typename iterator_traits<_ForwardIterator>::value_type __v;
  return std::adjacent_find(std::move(__first), std::move(__last), __equal_to<__v>());
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ALGORITHM_ADJACENT_FIND_H
