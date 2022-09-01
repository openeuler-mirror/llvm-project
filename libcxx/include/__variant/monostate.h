// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___VARIANT_MONOSTATE_H
#define _LIBCUDACXX___VARIANT_MONOSTATE_H

#include <__config>
#include <__functional/hash.h>
#include <cstddef>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 14

struct _LIBCUDACXX_TEMPLATE_VIS monostate {};

inline _LIBCUDACXX_INLINE_VISIBILITY
constexpr bool operator<(monostate, monostate) noexcept { return false; }

inline _LIBCUDACXX_INLINE_VISIBILITY
constexpr bool operator>(monostate, monostate) noexcept { return false; }

inline _LIBCUDACXX_INLINE_VISIBILITY
constexpr bool operator<=(monostate, monostate) noexcept { return true; }

inline _LIBCUDACXX_INLINE_VISIBILITY
constexpr bool operator>=(monostate, monostate) noexcept { return true; }

inline _LIBCUDACXX_INLINE_VISIBILITY
constexpr bool operator==(monostate, monostate) noexcept { return true; }

inline _LIBCUDACXX_INLINE_VISIBILITY
constexpr bool operator!=(monostate, monostate) noexcept { return false; }

template <>
struct _LIBCUDACXX_TEMPLATE_VIS hash<monostate> {
  using argument_type = monostate;
  using result_type = size_t;

  inline _LIBCUDACXX_INLINE_VISIBILITY
  result_type operator()(const argument_type&) const _NOEXCEPT {
    return 66740831; // return a fundamentally attractive random value.
  }
};

#endif // _LIBCUDACXX_STD_VER > 14

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___VARIANT_MONOSTATE_H
