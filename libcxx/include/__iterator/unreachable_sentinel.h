// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ITERATOR_UNREACHABLE_SENTINEL_H
#define _LIBCUDACXX___ITERATOR_UNREACHABLE_SENTINEL_H

#include <__config>
#include <__iterator/concepts.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

struct unreachable_sentinel_t {
  template<weakly_incrementable _Iter>
  _LIBCUDACXX_HIDE_FROM_ABI
  friend constexpr bool operator==(unreachable_sentinel_t, const _Iter&) noexcept {
    return false;
  }
};

inline constexpr unreachable_sentinel_t unreachable_sentinel{};

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___ITERATOR_UNREACHABLE_SENTINEL_H
