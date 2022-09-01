//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_IS_POLYMORPHIC_H
#define _LIBCUDACXX___TYPE_TRAITS_IS_POLYMORPHIC_H

#include <__config>
#include <__type_traits/integral_constant.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp>
struct _LIBCUDACXX_TEMPLATE_VIS is_polymorphic
    : public integral_constant<bool, __is_polymorphic(_Tp)> {};

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp>
inline constexpr bool is_polymorphic_v = __is_polymorphic(_Tp);
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_IS_POLYMORPHIC_H
