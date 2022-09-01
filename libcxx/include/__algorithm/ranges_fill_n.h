//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_RANGES_FILL_N_H
#define _LIBCUDACXX___ALGORITHM_RANGES_FILL_N_H

#include <__config>
#include <__iterator/concepts.h>
#include <__iterator/incrementable_traits.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCUDACXX_STD_VER > 17 && !defined(_LIBCUDACXX_HAS_NO_INCOMPLETE_RANGES)

_LIBCUDACXX_BEGIN_NAMESPACE_STD

namespace ranges {
namespace __fill_n {
struct __fn {
  template <class _Type, output_iterator<const _Type&> _Iter>
  _LIBCUDACXX_HIDE_FROM_ABI constexpr
  _Iter operator()(_Iter __first, iter_difference_t<_Iter> __n, const _Type& __value) const {
    for (; __n != 0; --__n) {
      *__first = __value;
      ++__first;
    }
    return __first;
  }
};
} // namespace __fill_n

inline namespace __cpo {
  inline constexpr auto fill_n = __fill_n::__fn{};
} // namespace __cpo
} // namespace ranges

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX_STD_VER > 17 && !defined(_LIBCUDACXX_HAS_NO_INCOMPLETE_RANGES)

#endif // _LIBCUDACXX___ALGORITHM_RANGES_FILL_N_H
