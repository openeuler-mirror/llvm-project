// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FORMAT_FORMAT_ERROR_H
#define _LIBCUDACXX___FORMAT_FORMAT_ERROR_H

#include <__config>
#include <stdexcept>

#ifdef _LIBCUDACXX_NO_EXCEPTIONS
#include <cstdlib>
#endif

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

class _LIBCUDACXX_EXCEPTION_ABI format_error : public runtime_error {
public:
  _LIBCUDACXX_HIDE_FROM_ABI explicit format_error(const string& __s)
      : runtime_error(__s) {}
  _LIBCUDACXX_HIDE_FROM_ABI explicit format_error(const char* __s)
      : runtime_error(__s) {}
  virtual ~format_error() noexcept;
};

_LIBCUDACXX_NORETURN inline _LIBCUDACXX_HIDE_FROM_ABI void
__throw_format_error(const char* __s) {
#ifndef _LIBCUDACXX_NO_EXCEPTIONS
  throw format_error(__s);
#else
  (void)__s;
  _CUDA_VSTD::abort();
#endif
}

#endif //_LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___FORMAT_FORMAT_ERROR_H
