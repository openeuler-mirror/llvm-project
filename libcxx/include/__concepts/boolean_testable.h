//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___CONCEPTS_BOOLEAN_TESTABLE_H
#define _LIBCUDACXX___CONCEPTS_BOOLEAN_TESTABLE_H

#include <__concepts/convertible_to.h>
#include <__config>
#include <__utility/forward.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

// [concepts.booleantestable]

template<class _Tp>
concept __boolean_testable_impl = convertible_to<_Tp, bool>;

template<class _Tp>
concept __boolean_testable = __boolean_testable_impl<_Tp> && requires(_Tp&& __t) {
  { !_CUDA_VSTD::forward<_Tp>(__t) } -> __boolean_testable_impl;
};

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___CONCEPTS_BOOLEAN_TESTABLE_H
