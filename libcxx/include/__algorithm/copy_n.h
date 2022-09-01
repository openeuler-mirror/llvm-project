//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_COPY_N_H
#define _LIBCUDACXX___ALGORITHM_COPY_N_H

#include <__algorithm/copy.h>
#include <__config>
#include <__iterator/iterator_traits.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template<class _InputIterator, class _Size, class _OutputIterator>
inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
typename enable_if
<
    __is_cpp17_input_iterator<_InputIterator>::value &&
   !__is_cpp17_random_access_iterator<_InputIterator>::value,
    _OutputIterator
>::type
copy_n(_InputIterator __first, _Size __orig_n, _OutputIterator __result)
{
    typedef decltype(_CUDA_VSTD::__convert_to_integral(__orig_n)) _IntegralSize;
    _IntegralSize __n = __orig_n;
    if (__n > 0)
    {
        *__result = *__first;
        ++__result;
        for (--__n; __n > 0; --__n)
        {
            ++__first;
            *__result = *__first;
            ++__result;
        }
    }
    return __result;
}

template<class _InputIterator, class _Size, class _OutputIterator>
inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
typename enable_if
<
    __is_cpp17_random_access_iterator<_InputIterator>::value,
    _OutputIterator
>::type
copy_n(_InputIterator __first, _Size __orig_n, _OutputIterator __result)
{
    typedef typename iterator_traits<_InputIterator>::difference_type difference_type;
    typedef decltype(_CUDA_VSTD::__convert_to_integral(__orig_n)) _IntegralSize;
    _IntegralSize __n = __orig_n;
    return _CUDA_VSTD::copy(__first, __first + difference_type(__n), __result);
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ALGORITHM_COPY_N_H
