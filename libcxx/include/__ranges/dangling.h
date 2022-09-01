// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___RANGES_DANGLING_H
#define _LIBCUDACXX___RANGES_DANGLING_H

#include <__config>
#include <__ranges/access.h>
#include <__ranges/concepts.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

namespace ranges {
struct dangling {
  dangling() = default;
  _LIBCUDACXX_HIDE_FROM_ABI constexpr dangling(auto&&...) noexcept {}
};

template <range _Rp>
using borrowed_iterator_t = _If<borrowed_range<_Rp>, iterator_t<_Rp>, dangling>;

// borrowed_subrange_t defined in <__ranges/subrange.h>
} // namespace ranges

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___RANGES_DANGLING_H
