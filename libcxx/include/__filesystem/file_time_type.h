// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FILESYSTEM_FILE_TIME_TYPE_H
#define _LIBCUDACXX___FILESYSTEM_FILE_TIME_TYPE_H

#include <__availability>
#include <__chrono/file_clock.h>
#include <__chrono/time_point.h>
#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#ifndef _LIBCUDACXX_CXX03_LANG

_LIBCUDACXX_BEGIN_NAMESPACE_FILESYSTEM

typedef chrono::time_point<_FilesystemClock> file_time_type;

_LIBCUDACXX_END_NAMESPACE_FILESYSTEM

#endif // _LIBCUDACXX_CXX03_LANG

#endif // _LIBCUDACXX___FILESYSTEM_FILE_TIME_TYPE_H
