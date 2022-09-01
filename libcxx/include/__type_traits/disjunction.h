//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_DISJUNCTION_H
#define _LIBCUDACXX___TYPE_TRAITS_DISJUNCTION_H

#include <__config>
#include <__type_traits/conditional.h>
#include <__type_traits/integral_constant.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <bool>
struct _OrImpl;

template <>
struct _OrImpl<true> {
  template <class _Res, class _First, class... _Rest>
  using _Result _LIBCUDACXX_NODEBUG =
      typename _OrImpl<!bool(_First::value) && sizeof...(_Rest) != 0>::template _Result<_First, _Rest...>;
};

template <>
struct _OrImpl<false> {
  template <class _Res, class...>
  using _Result = _Res;
};

template <class... _Args>
using _Or _LIBCUDACXX_NODEBUG = typename _OrImpl<sizeof...(_Args) != 0>::template _Result<false_type, _Args...>;

#if _LIBCUDACXX_STD_VER > 14

template <class... _Args>
struct disjunction : _Or<_Args...> {};

template <class... _Args>
inline constexpr bool disjunction_v = _Or<_Args...>::value;

#endif // _LIBCUDACXX_STD_VER > 14

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_DISJUNCTION_H
