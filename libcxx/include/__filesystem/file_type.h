// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FILESYSTEM_FILE_TYPE_H
#define _LIBCUDACXX___FILESYSTEM_FILE_TYPE_H

#include <__availability>
#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#ifndef _LIBCUDACXX_CXX03_LANG

_LIBCUDACXX_BEGIN_NAMESPACE_FILESYSTEM

// On Windows, the library never identifies files as block, character, fifo
// or socket.
enum class _LIBCUDACXX_ENUM_VIS file_type : signed char {
  none = 0,
  not_found = -1,
  regular = 1,
  directory = 2,
  symlink = 3,
  block = 4,
  character = 5,
  fifo = 6,
  socket = 7,
  unknown = 8
};

_LIBCUDACXX_END_NAMESPACE_FILESYSTEM

#endif // _LIBCUDACXX_CXX03_LANG

#endif // _LIBCUDACXX___FILESYSTEM_FILE_TYPE_H
