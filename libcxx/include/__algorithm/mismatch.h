// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_MISMATCH_H
#define _LIBCUDACXX___ALGORITHM_MISMATCH_H

#include <__algorithm/comp.h>
#include <__config>
#include <__iterator/iterator_traits.h>
#include <__utility/pair.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _InputIterator1, class _InputIterator2, class _BinaryPredicate>
_LIBCUDACXX_NODISCARD_EXT inline _LIBCUDACXX_INLINE_VISIBILITY
    _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1, _InputIterator2 __first2, _BinaryPredicate __pred) {
  for (; __first1 != __last1; ++__first1, (void)++__first2)
    if (!__pred(*__first1, *__first2))
      break;
  return pair<_InputIterator1, _InputIterator2>(__first1, __first2);
}

template <class _InputIterator1, class _InputIterator2>
_LIBCUDACXX_NODISCARD_EXT inline _LIBCUDACXX_INLINE_VISIBILITY
    _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1, _InputIterator2 __first2) {
  typedef typename iterator_traits<_InputIterator1>::value_type __v1;
  typedef typename iterator_traits<_InputIterator2>::value_type __v2;
  return _CUDA_VSTD::mismatch(__first1, __last1, __first2, __equal_to<__v1, __v2>());
}

#if _LIBCUDACXX_STD_VER > 11
template <class _InputIterator1, class _InputIterator2, class _BinaryPredicate>
_LIBCUDACXX_NODISCARD_EXT inline _LIBCUDACXX_INLINE_VISIBILITY
    _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1, _InputIterator2 __first2, _InputIterator2 __last2,
             _BinaryPredicate __pred) {
  for (; __first1 != __last1 && __first2 != __last2; ++__first1, (void)++__first2)
    if (!__pred(*__first1, *__first2))
      break;
  return pair<_InputIterator1, _InputIterator2>(__first1, __first2);
}

template <class _InputIterator1, class _InputIterator2>
_LIBCUDACXX_NODISCARD_EXT inline _LIBCUDACXX_INLINE_VISIBILITY
    _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 pair<_InputIterator1, _InputIterator2>
    mismatch(_InputIterator1 __first1, _InputIterator1 __last1, _InputIterator2 __first2, _InputIterator2 __last2) {
  typedef typename iterator_traits<_InputIterator1>::value_type __v1;
  typedef typename iterator_traits<_InputIterator2>::value_type __v2;
  return _CUDA_VSTD::mismatch(__first1, __last1, __first2, __last2, __equal_to<__v1, __v2>());
}
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ALGORITHM_MISMATCH_H
