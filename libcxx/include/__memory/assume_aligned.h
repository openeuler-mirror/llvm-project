// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___MEMORY_ASSUME_ALIGNED_H
#define _LIBCUDACXX___MEMORY_ASSUME_ALIGNED_H

#include <__assert>
#include <__config>
#include <cstddef>
#include <cstdint>
#include <type_traits> // for is_constant_evaluated()

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

template <size_t _Np, class _Tp>
[[nodiscard]]
_LIBCUDACXX_HIDE_FROM_ABI
constexpr _Tp* assume_aligned(_Tp* __ptr) {
  static_assert(_Np != 0 && (_Np & (_Np - 1)) == 0,
    "std::assume_aligned<N>(p) requires N to be a power of two");

  if (is_constant_evaluated()) {
    return __ptr;
  } else {
    _LIBCUDACXX_ASSERT(reinterpret_cast<uintptr_t>(__ptr) % _Np == 0, "Alignment assumption is violated");
    return static_cast<_Tp*>(__builtin_assume_aligned(__ptr, _Np));
  }
}

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___MEMORY_ASSUME_ALIGNED_H
