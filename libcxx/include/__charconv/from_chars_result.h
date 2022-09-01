// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___CHARCONV_FROM_CHARS_RESULT_H
#define _LIBCUDACXX___CHARCONV_FROM_CHARS_RESULT_H

#include <__config>
#include <__errc>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#ifndef _LIBCUDACXX_CXX03_LANG

struct _LIBCUDACXX_TYPE_VIS from_chars_result
{
    const char* ptr;
    errc ec;
#  if _LIBCUDACXX_STD_VER > 17
    _LIBCUDACXX_HIDE_FROM_ABI friend bool operator==(const from_chars_result&, const from_chars_result&) = default;
#  endif
};

#endif // _LIBCUDACXX_CXX03_LANG

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___CHARCONV_FROM_CHARS_RESULT_H
