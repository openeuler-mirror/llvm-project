//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_IS_UNSIGNED_H
#define _LIBCUDACXX___TYPE_TRAITS_IS_UNSIGNED_H

#include <__config>
#include <__type_traits/integral_constant.h>
#include <__type_traits/is_arithmetic.h>
#include <__type_traits/is_integral.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

// Before AppleClang 14, __is_unsigned returned true for enums with signed underlying type.
#if __has_builtin(__is_unsigned) && !(defined(_LIBCUDACXX_APPLE_CLANG_VER) && _LIBCUDACXX_APPLE_CLANG_VER < 1400)

template<class _Tp>
struct _LIBCUDACXX_TEMPLATE_VIS is_unsigned : _BoolConstant<__is_unsigned(_Tp)> { };

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp>
inline constexpr bool is_unsigned_v = __is_unsigned(_Tp);
#endif

#else // __has_builtin(__is_unsigned)

template <class _Tp, bool = is_integral<_Tp>::value>
struct __LIBCUDACXX_is_unsigned_impl : public _BoolConstant<(_Tp(0) < _Tp(-1))> {};

template <class _Tp>
struct __LIBCUDACXX_is_unsigned_impl<_Tp, false> : public false_type {};  // floating point

template <class _Tp, bool = is_arithmetic<_Tp>::value>
struct __LIBCUDACXX_is_unsigned : public __LIBCUDACXX_is_unsigned_impl<_Tp> {};

template <class _Tp> struct __LIBCUDACXX_is_unsigned<_Tp, false> : public false_type {};

template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS is_unsigned : public __LIBCUDACXX_is_unsigned<_Tp> {};

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp>
inline constexpr bool is_unsigned_v = is_unsigned<_Tp>::value;
#endif

#endif // __has_builtin(__is_unsigned)

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_IS_UNSIGNED_H
