//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_NEGATION_H
#define _LIBCUDACXX___TYPE_TRAITS_NEGATION_H

#include <__config>
#include <__type_traits/integral_constant.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Pred>
struct _Not : _BoolConstant<!_Pred::value> {};

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp>
struct negation : _Not<_Tp> {};
template<class _Tp>
inline constexpr bool negation_v = negation<_Tp>::value;
#endif // _LIBCUDACXX_STD_VER > 14

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_NEGATION_H
