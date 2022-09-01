//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_IS_LITERAL_TYPE
#define _LIBCUDACXX___TYPE_TRAITS_IS_LITERAL_TYPE

#include <__config>
#include <__type_traits/integral_constant.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER <= 17 || defined(_LIBCUDACXX_ENABLE_CXX20_REMOVED_TYPE_TRAITS)
template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS _LIBCUDACXX_DEPRECATED_IN_CXX17 is_literal_type
    : public integral_constant<bool, __is_literal_type(_Tp)>
    {};

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp>
_LIBCUDACXX_DEPRECATED_IN_CXX17 inline constexpr bool is_literal_type_v = is_literal_type<_Tp>::value;
#endif // _LIBCUDACXX_STD_VER > 14
#endif // _LIBCUDACXX_STD_VER <= 17 || defined(_LIBCUDACXX_ENABLE_CXX20_REMOVED_TYPE_TRAITS)

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_IS_LITERAL_TYPE
