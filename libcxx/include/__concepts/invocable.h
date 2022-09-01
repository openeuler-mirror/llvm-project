//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___CONCEPTS_INVOCABLE_H
#define _LIBCUDACXX___CONCEPTS_INVOCABLE_H

#include <__config>
#include <__functional/invoke.h>
#include <__utility/forward.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

// [concept.invocable]

template<class _Fn, class... _Args>
concept invocable = requires(_Fn&& __fn, _Args&&... __args) {
  _CUDA_VSTD::invoke(_CUDA_VSTD::forward<_Fn>(__fn), _CUDA_VSTD::forward<_Args>(__args)...); // not required to be equality preserving
};

// [concept.regular.invocable]

template<class _Fn, class... _Args>
concept regular_invocable = invocable<_Fn, _Args...>;

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___CONCEPTS_INVOCABLE_H
