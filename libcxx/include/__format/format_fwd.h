// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FORMAT_FORMAT_FWD_H
#define _LIBCUDACXX___FORMAT_FORMAT_FWD_H

#include <__availability>
#include <__config>
#include <__iterator/concepts.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

template <class _Context>
class _LIBCUDACXX_TEMPLATE_VIS _LIBCUDACXX_AVAILABILITY_FORMAT basic_format_arg;

template <class _OutIt, class _CharT>
  requires output_iterator<_OutIt, const _CharT&>
class _LIBCUDACXX_TEMPLATE_VIS _LIBCUDACXX_AVAILABILITY_FORMAT basic_format_context;

template <class _Tp, class _CharT = char>
struct _LIBCUDACXX_TEMPLATE_VIS _LIBCUDACXX_AVAILABILITY_FORMAT formatter;

#endif //_LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___FORMAT_FORMAT_FWD_H
