// -*- C++ -*-
//===---------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===---------------------------------------------------------------------===//

#ifndef _LIBCUDACXX_FWD_STRING_VIEW_H
#define _LIBCUDACXX_FWD_STRING_VIEW_H

#include <__config>
#include <iosfwd> // char_traits

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template<class _CharT, class _Traits = char_traits<_CharT> >
class _LIBCUDACXX_TEMPLATE_VIS basic_string_view;

typedef basic_string_view<char>     string_view;
#ifndef _LIBCUDACXX_HAS_NO_CHAR8_T
typedef basic_string_view<char8_t>  u8string_view;
#endif
typedef basic_string_view<char16_t> u16string_view;
typedef basic_string_view<char32_t> u32string_view;
#ifndef _LIBCUDACXX_HAS_NO_WIDE_CHARACTERS
typedef basic_string_view<wchar_t>  wstring_view;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX_FWD_STRING_VIEW_H
