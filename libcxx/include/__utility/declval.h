//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___UTILITY_DECLVAL_H
#define _LIBCUDACXX___UTILITY_DECLVAL_H

#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

// Suppress deprecation notice for volatile-qualified return type resulting
// from volatile-qualified types _Tp.
_LIBCUDACXX_SUPPRESS_DEPRECATED_PUSH
template <class _Tp>
_Tp&& __declval(int);
template <class _Tp>
_Tp __declval(long);
_LIBCUDACXX_SUPPRESS_DEPRECATED_POP

template <class _Tp>
decltype(__declval<_Tp>(0)) declval() _NOEXCEPT;

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___UTILITY_DECLVAL_H
