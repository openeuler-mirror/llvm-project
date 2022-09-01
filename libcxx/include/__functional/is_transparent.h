// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FUNCTIONAL_IS_TRANSPARENT
#define _LIBCUDACXX___FUNCTIONAL_IS_TRANSPARENT

#include <__config>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 11

template <class _Tp, class, class = void>
struct __is_transparent : false_type {};

template <class _Tp, class _Up>
struct __is_transparent<_Tp, _Up,
                        typename __void_t<typename _Tp::is_transparent>::type>
   : true_type {};

#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___FUNCTIONAL_IS_TRANSPARENT
