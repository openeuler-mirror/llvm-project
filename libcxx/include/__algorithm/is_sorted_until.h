//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_IS_SORTED_UNTIL_H
#define _LIBCUDACXX___ALGORITHM_IS_SORTED_UNTIL_H

#include <__algorithm/comp.h>
#include <__algorithm/comp_ref_type.h>
#include <__config>
#include <__iterator/iterator_traits.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Compare, class _ForwardIterator>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 _ForwardIterator
__is_sorted_until(_ForwardIterator __first, _ForwardIterator __last, _Compare __comp)
{
    if (__first != __last)
    {
        _ForwardIterator __i = __first;
        while (++__i != __last)
        {
            if (__comp(*__i, *__first))
                return __i;
            __first = __i;
        }
    }
    return __last;
}

template <class _ForwardIterator, class _Compare>
_LIBCUDACXX_NODISCARD_EXT inline _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 _ForwardIterator
is_sorted_until(_ForwardIterator __first, _ForwardIterator __last, _Compare __comp)
{
    typedef typename __comp_ref_type<_Compare>::type _Comp_ref;
    return _CUDA_VSTD::__is_sorted_until<_Comp_ref>(__first, __last, __comp);
}

template<class _ForwardIterator>
_LIBCUDACXX_NODISCARD_EXT inline _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 _ForwardIterator
is_sorted_until(_ForwardIterator __first, _ForwardIterator __last)
{
    return _CUDA_VSTD::is_sorted_until(__first, __last, __less<typename iterator_traits<_ForwardIterator>::value_type>());
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ALGORITHM_IS_SORTED_UNTIL_H
