//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_CONJUNCTION_H
#define _LIBCUDACXX___TYPE_TRAITS_CONJUNCTION_H

#include <__config>
#include <__type_traits/conditional.h>
#include <__type_traits/enable_if.h>
#include <__type_traits/integral_constant.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 14

template <class _Arg, class... _Args>
struct __conjunction_impl {
  using type = conditional_t<!bool(_Arg::value), _Arg, typename __conjunction_impl<_Args...>::type>;
};

template <class _Arg>
struct __conjunction_impl<_Arg> {
  using type = _Arg;
};

template <class... _Args>
struct conjunction : __conjunction_impl<true_type, _Args...>::type {};

template<class... _Args>
inline constexpr bool conjunction_v = conjunction<_Args...>::value;

#endif // _LIBCUDACXX_STD_VER > 14

template <class...>
using __expand_to_true = true_type;

template <class... _Pred>
__expand_to_true<__enable_if_t<_Pred::value>...> __and_helper(int);

template <class...>
false_type __and_helper(...);

template <class... _Pred>
using _And _LIBCUDACXX_NODEBUG = decltype(__and_helper<_Pred...>(0));

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_CONJUNCTION_H
