//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___UTILITY_EXCHANGE_H
#define _LIBCUDACXX___UTILITY_EXCHANGE_H

#include <__config>
#include <__utility/forward.h>
#include <__utility/move.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 11
template<class _T1, class _T2 = _T1>
inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
_T1 exchange(_T1& __obj, _T2&& __new_value)
    noexcept(is_nothrow_move_constructible<_T1>::value && is_nothrow_assignable<_T1&, _T2>::value)
{
    _T1 __old_value = _CUDA_VSTD::move(__obj);
    __obj = _CUDA_VSTD::forward<_T2>(__new_value);
    return __old_value;
}
#endif // _LIBCUDACXX_STD_VER > 11

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___UTILITY_EXCHANGE_H
