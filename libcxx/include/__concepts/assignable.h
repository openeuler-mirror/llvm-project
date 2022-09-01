//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___CONCEPTS_ASSIGNABLE_H
#define _LIBCUDACXX___CONCEPTS_ASSIGNABLE_H

#include <__concepts/common_reference_with.h>
#include <__concepts/same_as.h>
#include <__config>
#include <__utility/forward.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

// [concept.assignable]

template<class _Lhs, class _Rhs>
concept assignable_from =
  is_lvalue_reference_v<_Lhs> &&
  common_reference_with<__make_const_lvalue_ref<_Lhs>, __make_const_lvalue_ref<_Rhs>> &&
  requires (_Lhs __lhs, _Rhs&& __rhs) {
    { __lhs = _CUDA_VSTD::forward<_Rhs>(__rhs) } -> same_as<_Lhs>;
  };

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___CONCEPTS_ASSIGNABLE_H
