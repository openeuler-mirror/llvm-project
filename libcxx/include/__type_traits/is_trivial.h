//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_IS_TRIVIAL_H
#define _LIBCUDACXX___TYPE_TRAITS_IS_TRIVIAL_H

#include <__config>
#include <__type_traits/integral_constant.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS is_trivial
#if __has_builtin(__is_trivial)
    : public integral_constant<bool, __is_trivial(_Tp)>
#else
    : integral_constant<bool, is_trivially_copyable<_Tp>::value &&
                                 is_trivially_default_constructible<_Tp>::value>
#endif
    {};

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp>
inline constexpr bool is_trivial_v = is_trivial<_Tp>::value;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_IS_TRIVIAL_H
