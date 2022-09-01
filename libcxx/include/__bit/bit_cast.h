// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___BIT_BIT_CAST_H
#define _LIBCUDACXX___BIT_BIT_CAST_H

#include <__config>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

template <class _ToType, class _FromType>
  requires(sizeof(_ToType) == sizeof(_FromType) &&
           is_trivially_copyable_v<_ToType> &&
           is_trivially_copyable_v<_FromType>)
_LIBCUDACXX_NODISCARD_EXT _LIBCUDACXX_HIDE_FROM_ABI constexpr _ToType bit_cast(const _FromType& __from) noexcept {
  return __builtin_bit_cast(_ToType, __from);
}

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___BIT_BIT_CAST_H
