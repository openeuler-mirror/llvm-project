// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___CHRONO_FILE_CLOCK_H
#define _LIBCUDACXX___CHRONO_FILE_CLOCK_H

#include <__availability>
#include <__chrono/duration.h>
#include <__chrono/system_clock.h>
#include <__chrono/time_point.h>
#include <__config>
#include <ratio>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#ifndef _LIBCUDACXX_CXX03_LANG
_LIBCUDACXX_BEGIN_NAMESPACE_FILESYSTEM
struct _FilesystemClock;
_LIBCUDACXX_END_NAMESPACE_FILESYSTEM
#endif // !_LIBCUDACXX_CXX03_LANG

#if _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_BEGIN_NAMESPACE_STD

namespace chrono
{

// [time.clock.file], type file_clock
using file_clock = _VSTD_FS::_FilesystemClock;

template<class _Duration>
using file_time = time_point<file_clock, _Duration>;

} // namespace chrono

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX_STD_VER > 17

#ifndef _LIBCUDACXX_CXX03_LANG
_LIBCUDACXX_BEGIN_NAMESPACE_FILESYSTEM
struct _FilesystemClock {
#if !defined(_LIBCUDACXX_HAS_NO_INT128)
  typedef __int128_t rep;
  typedef nano period;
#else
  typedef long long rep;
  typedef nano period;
#endif

  typedef chrono::duration<rep, period> duration;
  typedef chrono::time_point<_FilesystemClock> time_point;

  _LIBCUDACXX_EXPORTED_FROM_ABI
  static _LIBCUDACXX_CONSTEXPR_AFTER_CXX11 const bool is_steady = false;

  _LIBCUDACXX_AVAILABILITY_FILESYSTEM _LIBCUDACXX_FUNC_VIS static time_point now() noexcept;

#if _LIBCUDACXX_STD_VER > 17
  template <class _Duration>
  _LIBCUDACXX_HIDE_FROM_ABI
  static chrono::sys_time<_Duration> to_sys(const chrono::file_time<_Duration>& __t) {
    return chrono::sys_time<_Duration>(__t.time_since_epoch());
  }

  template <class _Duration>
  _LIBCUDACXX_HIDE_FROM_ABI
  static chrono::file_time<_Duration> from_sys(const chrono::sys_time<_Duration>& __t) {
    return chrono::file_time<_Duration>(__t.time_since_epoch());
  }
#endif // _LIBCUDACXX_STD_VER > 17
};
_LIBCUDACXX_END_NAMESPACE_FILESYSTEM
#endif // !_LIBCUDACXX_CXX03_LANG

#endif // _LIBCUDACXX___CHRONO_FILE_CLOCK_H
