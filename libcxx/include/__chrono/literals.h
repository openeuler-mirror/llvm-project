// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___CHRONO_LITERALS_H
#define _LIBCUDACXX___CHRONO_LITERALS_H

#include <__chrono/day.h>
#include <__chrono/year.h>
#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_BEGIN_NAMESPACE_STD

inline namespace literals
{
  inline namespace chrono_literals
  {
    _LIBCUDACXX_HIDE_FROM_ABI constexpr chrono::day operator ""d(unsigned long long __d) noexcept
    {
        return chrono::day(static_cast<unsigned>(__d));
    }

    _LIBCUDACXX_HIDE_FROM_ABI constexpr chrono::year operator ""y(unsigned long long __y) noexcept
    {
        return chrono::year(static_cast<int>(__y));
    }
} // namespace chrono_literals
} // namespace literals

namespace chrono { // hoist the literals into namespace std::chrono
   using namespace literals::chrono_literals;
} // namespace chrono

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX_STD_VER > 17

#endif // _LIBCUDACXX___CHRONO_LITERALS_H
