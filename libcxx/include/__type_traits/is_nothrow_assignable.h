//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_IS_NOTHROW_ASSIGNABLE_H
#define _LIBCUDACXX___TYPE_TRAITS_IS_NOTHROW_ASSIGNABLE_H

#include <__config>
#include <__type_traits/add_const.h>
#include <__type_traits/integral_constant.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if __has_builtin(__is_nothrow_assignable)

template <class _Tp, class _Arg>
struct _LIBCUDACXX_TEMPLATE_VIS is_nothrow_assignable
    : public integral_constant<bool, __is_nothrow_assignable(_Tp, _Arg)> {};

#else

template <bool, class _Tp, class _Arg> struct __LIBCUDACXX_is_nothrow_assignable;

template <class _Tp, class _Arg>
struct __LIBCUDACXX_is_nothrow_assignable<false, _Tp, _Arg>
    : public false_type
{
};

template <class _Tp, class _Arg>
struct __LIBCUDACXX_is_nothrow_assignable<true, _Tp, _Arg>
    : public integral_constant<bool, noexcept(declval<_Tp>() = declval<_Arg>()) >
{
};

template <class _Tp, class _Arg>
struct _LIBCUDACXX_TEMPLATE_VIS is_nothrow_assignable
    : public __LIBCUDACXX_is_nothrow_assignable<is_assignable<_Tp, _Arg>::value, _Tp, _Arg>
{
};

#endif // __has_builtin(__is_nothrow_assignable)

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp, class _Arg>
inline constexpr bool is_nothrow_assignable_v = is_nothrow_assignable<_Tp, _Arg>::value;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_IS_NOTHROW_ASSIGNABLE_H
