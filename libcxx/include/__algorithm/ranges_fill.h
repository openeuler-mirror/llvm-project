//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_RANGES_FILL_H
#define _LIBCUDACXX___ALGORITHM_RANGES_FILL_H

#include <__algorithm/ranges_fill_n.h>
#include <__config>
#include <__iterator/concepts.h>
#include <__ranges/access.h>
#include <__ranges/concepts.h>
#include <__ranges/dangling.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCUDACXX_STD_VER > 17 && !defined(_LIBCUDACXX_HAS_NO_INCOMPLETE_RANGES)

_LIBCUDACXX_BEGIN_NAMESPACE_STD

namespace ranges {
namespace __fill {
struct __fn {
  template <class _Type, output_iterator<const _Type&> _Iter, sentinel_for<_Iter> _Sent>
  _LIBCUDACXX_HIDE_FROM_ABI constexpr
  _Iter operator()(_Iter __first, _Sent __last, const _Type& __value) const {
    if constexpr(random_access_iterator<_Iter> && sized_sentinel_for<_Sent, _Iter>) {
      return ranges::fill_n(__first, __last - __first, __value);
    } else {
      for (; __first != __last; ++__first)
        *__first = __value;
      return __first;
    }
  }

  template <class _Type, output_range<const _Type&> _Range>
  _LIBCUDACXX_HIDE_FROM_ABI constexpr
  borrowed_iterator_t<_Range> operator()(_Range&& __range, const _Type& __value) const {
    return (*this)(ranges::begin(__range), ranges::end(__range), __value);
  }
};
} // namespace __fill

inline namespace __cpo {
  inline constexpr auto fill = __fill::__fn{};
} // namespace __cpo
} // namespace ranges

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX_STD_VER > 17 && !defined(_LIBCUDACXX_HAS_NO_INCOMPLETE_RANGES)

#endif // _LIBCUDACXX___ALGORITHM_RANGES_FILL_H
