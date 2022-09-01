//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_IS_MEMBER_OBJECT_POINTER_H
#define _LIBCUDACXX___TYPE_TRAITS_IS_MEMBER_OBJECT_POINTER_H

#include <__config>
#include <__type_traits/integral_constant.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if __has_builtin(__is_member_object_pointer)

template<class _Tp>
struct _LIBCUDACXX_TEMPLATE_VIS is_member_object_pointer
    : _BoolConstant<__is_member_object_pointer(_Tp)> { };

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp>
inline constexpr bool is_member_object_pointer_v = __is_member_object_pointer(_Tp);
#endif

#else // __has_builtin(__is_member_object_pointer)

template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS is_member_object_pointer
    : public _BoolConstant< __LIBCUDACXX_is_member_pointer<typename remove_cv<_Tp>::type>::__is_obj >  {};

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp>
inline constexpr bool is_member_object_pointer_v = is_member_object_pointer<_Tp>::value;
#endif

#endif // __has_builtin(__is_member_object_pointer)

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_IS_MEMBER_FUNCTION_POINTER_H
