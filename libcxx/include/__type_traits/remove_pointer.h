//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_REMOVE_POINTER_H
#define _LIBCUDACXX___TYPE_TRAITS_REMOVE_POINTER_H

#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS remove_pointer                      {typedef _LIBCUDACXX_NODEBUG _Tp type;};
template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS remove_pointer<_Tp*>                {typedef _LIBCUDACXX_NODEBUG _Tp type;};
template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS remove_pointer<_Tp* const>          {typedef _LIBCUDACXX_NODEBUG _Tp type;};
template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS remove_pointer<_Tp* volatile>       {typedef _LIBCUDACXX_NODEBUG _Tp type;};
template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS remove_pointer<_Tp* const volatile> {typedef _LIBCUDACXX_NODEBUG _Tp type;};

#if _LIBCUDACXX_STD_VER > 11
template <class _Tp> using remove_pointer_t = typename remove_pointer<_Tp>::type;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_REMOVE_POINTER_H
