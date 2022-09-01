//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___MEMORY_SWAP_ALLOCATOR_H
#define _LIBCUDACXX___MEMORY_SWAP_ALLOCATOR_H

#include <__config>
#include <__memory/allocator_traits.h>
#include <__type_traits/integral_constant.h>
#include <__utility/swap.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <typename _Alloc>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX11 void __swap_allocator(_Alloc& __a1, _Alloc& __a2, true_type)
#if _LIBCUDACXX_STD_VER > 11
    _NOEXCEPT
#else
    _NOEXCEPT_(__is_nothrow_swappable<_Alloc>::value)
#endif
{
  using _CUDA_VSTD::swap;
  swap(__a1, __a2);
}

template <typename _Alloc>
inline _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX11 void
__swap_allocator(_Alloc&, _Alloc&, false_type) _NOEXCEPT {}

template <typename _Alloc>
inline _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX11 void __swap_allocator(_Alloc& __a1, _Alloc& __a2)
#if _LIBCUDACXX_STD_VER > 11
    _NOEXCEPT
#else
    _NOEXCEPT_(__is_nothrow_swappable<_Alloc>::value)
#endif
{
  _CUDA_VSTD::__swap_allocator(
      __a1, __a2, integral_constant<bool, allocator_traits<_Alloc>::propagate_on_container_swap::value>());
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___MEMORY_SWAP_ALLOCATOR_H
