//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_REMOVE_IF_H
#define _LIBCUDACXX___ALGORITHM_REMOVE_IF_H

#include <__algorithm/find_if.h>
#include <__config>
#include <__utility/move.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _ForwardIterator, class _Predicate>
_LIBCUDACXX_NODISCARD_EXT _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 _ForwardIterator
remove_if(_ForwardIterator __first, _ForwardIterator __last, _Predicate __pred)
{
    __first = _CUDA_VSTD::find_if<_ForwardIterator, _Predicate&>(__first, __last, __pred);
    if (__first != __last)
    {
        _ForwardIterator __i = __first;
        while (++__i != __last)
        {
            if (!__pred(*__i))
            {
                *__first = _CUDA_VSTD::move(*__i);
                ++__first;
            }
        }
    }
    return __first;
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ALGORITHM_REMOVE_IF_H
