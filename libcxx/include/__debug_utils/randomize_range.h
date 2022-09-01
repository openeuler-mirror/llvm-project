//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___LIBCXX_DEBUG_RANDOMIZE_RANGE_H
#define _LIBCUDACXX___LIBCXX_DEBUG_RANDOMIZE_RANGE_H

#include <__config>

#ifdef _LIBCUDACXX_DEBUG_RANDOMIZE_UNSPECIFIED_STABILITY
#  include <__algorithm/shuffle.h>
#  include <__type_traits/is_constant_evaluated.h>
#endif

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _AlgPolicy, class _Iterator, class _Sentinel>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX11
void __debug_randomize_range(_Iterator __first, _Sentinel __last) {
#ifdef _LIBCUDACXX_DEBUG_RANDOMIZE_UNSPECIFIED_STABILITY
#  ifdef _LIBCUDACXX_CXX03_LANG
#    error Support for unspecified stability is only for C++11 and higher
#  endif

  if (!__LIBCUDACXX_is_constant_evaluated())
    std::__shuffle<_AlgPolicy>(__first, __last, __LIBCUDACXX_debug_randomizer());
#else
  (void)__first;
  (void)__last;
#endif
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___LIBCXX_DEBUG_RANDOMIZE_RANGE_H
