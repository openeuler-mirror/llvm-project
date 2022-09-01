//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_IS_TRIVIALLY_CONSTRUCTIBLE_H
#define _LIBCUDACXX___TYPE_TRAITS_IS_TRIVIALLY_CONSTRUCTIBLE_H

#include <__config>
#include <__type_traits/integral_constant.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp, class... _Args>
struct _LIBCUDACXX_TEMPLATE_VIS is_trivially_constructible
    : integral_constant<bool, __is_trivially_constructible(_Tp, _Args...)>
{
};

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp, class... _Args>
inline constexpr bool is_trivially_constructible_v = is_trivially_constructible<_Tp, _Args...>::value;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_IS_TRIVIALLY_CONSTRUCTIBLE_H
