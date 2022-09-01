//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_HALF_POSITIVE_H
#define _LIBCUDACXX___ALGORITHM_HALF_POSITIVE_H

#include <__config>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

// Perform division by two quickly for positive integers (llvm.org/PR39129)

template <typename _Integral>
_LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR
typename enable_if
<
    is_integral<_Integral>::value,
    _Integral
>::type
__half_positive(_Integral __value)
{
    return static_cast<_Integral>(static_cast<typename make_unsigned<_Integral>::type>(__value) / 2);
}

template <typename _Tp>
_LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR
typename enable_if
<
    !is_integral<_Tp>::value,
    _Tp
>::type
__half_positive(_Tp __value)
{
    return __value / 2;
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ALGORITHM_HALF_POSITIVE_H
