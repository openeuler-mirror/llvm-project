//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___MEMORY_ALLOCATE_AT_LEAST_H
#define _LIBCUDACXX___MEMORY_ALLOCATE_AT_LEAST_H

#include <__config>
#include <__memory/allocator_traits.h>
#include <cstddef>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 20
template <class _Pointer>
struct allocation_result {
  _Pointer ptr;
  size_t count;
};

template <class _Alloc>
[[nodiscard]] _LIBCUDACXX_HIDE_FROM_ABI constexpr
allocation_result<typename allocator_traits<_Alloc>::pointer> allocate_at_least(_Alloc& __alloc, size_t __n) {
  if constexpr (requires { __alloc.allocate_at_least(__n); }) {
    return __alloc.allocate_at_least(__n);
  } else {
    return {__alloc.allocate(__n), __n};
  }
}

template <class _Alloc>
[[nodiscard]] _LIBCUDACXX_HIDE_FROM_ABI constexpr
auto __allocate_at_least(_Alloc& __alloc, size_t __n) {
  return std::allocate_at_least(__alloc, __n);
}
#else
template <class _Pointer>
struct __allocation_result {
  _Pointer ptr;
  size_t count;
};

template <class _Alloc>
_LIBCUDACXX_NODISCARD _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR
__allocation_result<typename allocator_traits<_Alloc>::pointer> __allocate_at_least(_Alloc& __alloc, size_t __n) {
  return {__alloc.allocate(__n), __n};
}

#endif // _LIBCUDACXX_STD_VER > 20

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___MEMORY_ALLOCATE_AT_LEAST_H
