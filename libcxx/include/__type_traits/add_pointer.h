//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_ADD_POINTER_H
#define _LIBCUDACXX___TYPE_TRAITS_ADD_POINTER_H

#include <__config>
#include <__type_traits/is_referenceable.h>
#include <__type_traits/is_same.h>
#include <__type_traits/remove_cv.h>
#include <__type_traits/remove_reference.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp,
        bool = __is_referenceable<_Tp>::value ||
                _IsSame<typename remove_cv<_Tp>::type, void>::value>
struct __add_pointer_impl
    {typedef _LIBCUDACXX_NODEBUG typename remove_reference<_Tp>::type* type;};
template <class _Tp> struct __add_pointer_impl<_Tp, false>
    {typedef _LIBCUDACXX_NODEBUG _Tp type;};

template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS add_pointer
    {typedef _LIBCUDACXX_NODEBUG typename __add_pointer_impl<_Tp>::type type;};

#if _LIBCUDACXX_STD_VER > 11
template <class _Tp> using add_pointer_t = typename add_pointer<_Tp>::type;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_ADD_POINTER_H
