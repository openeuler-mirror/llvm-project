// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
#ifndef _LIBCUDACXX___THREAD_TIMED_BACKOFF_POLICY_H
#define _LIBCUDACXX___THREAD_TIMED_BACKOFF_POLICY_H

#include <__config>

#ifndef _LIBCUDACXX_HAS_NO_THREADS

#  include <__chrono/duration.h>
#  include <__threading_support>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

struct __LIBCUDACXX_timed_backoff_policy {
  _LIBCUDACXX_INLINE_VISIBILITY
  bool operator()(chrono::nanoseconds __elapsed) const
  {
      if(__elapsed > chrono::milliseconds(128))
          __LIBCUDACXX_thread_sleep_for(chrono::milliseconds(8));
      else if(__elapsed > chrono::microseconds(64))
          __LIBCUDACXX_thread_sleep_for(__elapsed / 2);
      else if(__elapsed > chrono::microseconds(4))
        __LIBCUDACXX_thread_yield();
      else
        {} // poll
      return false;
  }
};

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX_HAS_NO_THREADS

#endif // _LIBCUDACXX___THREAD_TIMED_BACKOFF_POLICY_H
