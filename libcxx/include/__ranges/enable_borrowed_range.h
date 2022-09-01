// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___RANGES_ENABLE_BORROWED_RANGE_H
#define _LIBCUDACXX___RANGES_ENABLE_BORROWED_RANGE_H

// These customization variables are used in <span> and <string_view>. The
// separate header is used to avoid including the entire <ranges> header in
// <span> and <string_view>.

#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

namespace ranges {

// [range.range], ranges

template <class>
inline constexpr bool enable_borrowed_range = false;

} // namespace ranges

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___RANGES_ENABLE_BORROWED_RANGE_H
