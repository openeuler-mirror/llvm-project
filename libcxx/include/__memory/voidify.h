// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___MEMORY_VOIDIFY_H
#define _LIBCUDACXX___MEMORY_VOIDIFY_H

#include <__config>
#include <__memory/addressof.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <typename _Tp>
_LIBCUDACXX_ALWAYS_INLINE _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17 void* __voidify(_Tp& __from) {
  // Cast away cv-qualifiers to allow modifying elements of a range through const iterators.
  return const_cast<void*>(static_cast<const volatile void*>(_CUDA_VSTD::addressof(__from)));
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___MEMORY_VOIDIFY_H
