//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_REPLACE_COPY_H
#define _LIBCUDACXX___ALGORITHM_REPLACE_COPY_H

#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _InputIterator, class _OutputIterator, class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
_OutputIterator
replace_copy(_InputIterator __first, _InputIterator __last, _OutputIterator __result,
             const _Tp& __old_value, const _Tp& __new_value)
{
    for (; __first != __last; ++__first, (void) ++__result)
        if (*__first == __old_value)
            *__result = __new_value;
        else
            *__result = *__first;
    return __result;
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ALGORITHM_REPLACE_COPY_H
