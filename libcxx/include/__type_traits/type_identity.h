//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_TYPE_IDENTITY_H
#define _LIBCUDACXX___TYPE_TRAITS_TYPE_IDENTITY_H

#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp>
struct __type_identity { typedef _Tp type; };

template <class _Tp>
using __type_identity_t _LIBCUDACXX_NODEBUG = typename __type_identity<_Tp>::type;

#if _LIBCUDACXX_STD_VER > 17
template<class _Tp> struct type_identity { typedef _Tp type; };
template<class _Tp> using type_identity_t = typename type_identity<_Tp>::type;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_TYPE_IDENTITY_H
