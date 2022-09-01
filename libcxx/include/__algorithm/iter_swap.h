//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_ITER_SWAP_H
#define _LIBCUDACXX___ALGORITHM_ITER_SWAP_H

#include <__config>
#include <__utility/declval.h>
#include <__utility/swap.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _ForwardIterator1, class _ForwardIterator2>
inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 void iter_swap(_ForwardIterator1 __a,
                                                                              _ForwardIterator2 __b)
    //                                  _NOEXCEPT_(_NOEXCEPT_(swap(*__a, *__b)))
    _NOEXCEPT_(_NOEXCEPT_(swap(*declval<_ForwardIterator1>(), *declval<_ForwardIterator2>()))) {
  swap(*__a, *__b);
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ALGORITHM_ITER_SWAP_H
