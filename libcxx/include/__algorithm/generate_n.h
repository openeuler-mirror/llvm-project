//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_GENERATE_N_H
#define _LIBCUDACXX___ALGORITHM_GENERATE_N_H

#include <__config>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _OutputIterator, class _Size, class _Generator>
inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
_OutputIterator
generate_n(_OutputIterator __first, _Size __orig_n, _Generator __gen)
{
    typedef decltype(_CUDA_VSTD::__convert_to_integral(__orig_n)) _IntegralSize;
    _IntegralSize __n = __orig_n;
    for (; __n > 0; ++__first, (void) --__n)
        *__first = __gen();
    return __first;
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ALGORITHM_GENERATE_N_H
