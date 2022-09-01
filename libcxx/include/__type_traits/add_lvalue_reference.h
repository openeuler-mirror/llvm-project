//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_ADD_LVALUE_REFERENCE_H
#define _LIBCUDACXX___TYPE_TRAITS_ADD_LVALUE_REFERENCE_H

#include <__config>
#include <__type_traits/is_referenceable.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp, bool = __is_referenceable<_Tp>::value> struct __add_lvalue_reference_impl            { typedef _LIBCUDACXX_NODEBUG _Tp  type; };
template <class _Tp                                       > struct __add_lvalue_reference_impl<_Tp, true> { typedef _LIBCUDACXX_NODEBUG _Tp& type; };

template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS add_lvalue_reference
{typedef _LIBCUDACXX_NODEBUG typename  __add_lvalue_reference_impl<_Tp>::type type;};

#if _LIBCUDACXX_STD_VER > 11
template <class _Tp> using add_lvalue_reference_t = typename add_lvalue_reference<_Tp>::type;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_ADD_LVALUE_REFERENCE_H
