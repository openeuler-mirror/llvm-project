// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FILESYSTEM_SPACE_INFO_H
#define _LIBCUDACXX___FILESYSTEM_SPACE_INFO_H

#include <__availability>
#include <__config>
#include <cstdint>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#ifndef _LIBCUDACXX_CXX03_LANG

_LIBCUDACXX_BEGIN_NAMESPACE_FILESYSTEM

_LIBCUDACXX_AVAILABILITY_FILESYSTEM_PUSH

struct _LIBCUDACXX_TYPE_VIS space_info {
  uintmax_t capacity;
  uintmax_t free;
  uintmax_t available;
};

_LIBCUDACXX_AVAILABILITY_FILESYSTEM_POP

_LIBCUDACXX_END_NAMESPACE_FILESYSTEM

#endif // _LIBCUDACXX_CXX03_LANG

#endif // _LIBCUDACXX___FILESYSTEM_SPACE_INFO_H
