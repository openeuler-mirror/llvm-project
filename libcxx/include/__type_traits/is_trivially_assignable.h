//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_IS_TRIVIALLY_ASSIGNABLE_H
#define _LIBCUDACXX___TYPE_TRAITS_IS_TRIVIALLY_ASSIGNABLE_H

#include <__config>
#include <__type_traits/integral_constant.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp, class _Arg>
struct is_trivially_assignable
    : integral_constant<bool, __is_trivially_assignable(_Tp, _Arg)>
{ };

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp, class _Arg>
inline constexpr bool is_trivially_assignable_v = is_trivially_assignable<_Tp, _Arg>::value;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_IS_TRIVIALLY_ASSIGNABLE_H
