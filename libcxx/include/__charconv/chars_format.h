// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___CHARCONV_CHARS_FORMAT_H
#define _LIBCUDACXX___CHARCONV_CHARS_FORMAT_H

#include <__config>
#include <__utility/to_underlying.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#ifndef _LIBCUDACXX_CXX03_LANG

enum class _LIBCUDACXX_ENUM_VIS chars_format
{
    scientific = 0x1,
    fixed = 0x2,
    hex = 0x4,
    general = fixed | scientific
};

inline _LIBCUDACXX_INLINE_VISIBILITY constexpr chars_format
operator~(chars_format __x) {
  return chars_format(~_CUDA_VSTD::__to_underlying(__x));
}

inline _LIBCUDACXX_INLINE_VISIBILITY constexpr chars_format
operator&(chars_format __x, chars_format __y) {
  return chars_format(_CUDA_VSTD::__to_underlying(__x) &
                      _CUDA_VSTD::__to_underlying(__y));
}

inline _LIBCUDACXX_INLINE_VISIBILITY constexpr chars_format
operator|(chars_format __x, chars_format __y) {
  return chars_format(_CUDA_VSTD::__to_underlying(__x) |
                      _CUDA_VSTD::__to_underlying(__y));
}

inline _LIBCUDACXX_INLINE_VISIBILITY constexpr chars_format
operator^(chars_format __x, chars_format __y) {
  return chars_format(_CUDA_VSTD::__to_underlying(__x) ^
                      _CUDA_VSTD::__to_underlying(__y));
}

inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX11 chars_format&
operator&=(chars_format& __x, chars_format __y) {
  __x = __x & __y;
  return __x;
}

inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX11 chars_format&
operator|=(chars_format& __x, chars_format __y) {
  __x = __x | __y;
  return __x;
}

inline _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX11 chars_format&
operator^=(chars_format& __x, chars_format __y) {
  __x = __x ^ __y;
  return __x;
}

#endif // _LIBCUDACXX_CXX03_LANG

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___CHARCONV_CHARS_FORMAT_H
