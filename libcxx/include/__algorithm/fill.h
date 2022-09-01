//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_FILL_H
#define _LIBCUDACXX___ALGORITHM_FILL_H

#include <__algorithm/fill_n.h>
#include <__config>
#include <__iterator/iterator_traits.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

// fill isn't specialized for std::memset, because the compiler already optimizes the loop to a call to std::memset.

template <class _ForwardIterator, class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
void
__fill(_ForwardIterator __first, _ForwardIterator __last, const _Tp& __value, forward_iterator_tag)
{
    for (; __first != __last; ++__first)
        *__first = __value;
}

template <class _RandomAccessIterator, class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
void
__fill(_RandomAccessIterator __first, _RandomAccessIterator __last, const _Tp& __value, random_access_iterator_tag)
{
    _CUDA_VSTD::fill_n(__first, __last - __first, __value);
}

template <class _ForwardIterator, class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
void
fill(_ForwardIterator __first, _ForwardIterator __last, const _Tp& __value)
{
    _CUDA_VSTD::__fill(__first, __last, __value, typename iterator_traits<_ForwardIterator>::iterator_category());
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ALGORITHM_FILL_H
