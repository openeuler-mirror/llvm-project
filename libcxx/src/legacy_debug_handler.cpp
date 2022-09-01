//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <__config>
#include <cstdio>
#include <cstdlib>
#include <string>

// This file defines the legacy default debug handler and related mechanisms
// to set it. This is for backwards ABI compatibility with code that has been
// using this debug handler previously.

_LIBCUDACXX_BEGIN_NAMESPACE_STD

struct _LIBCUDACXX_TEMPLATE_VIS __LIBCUDACXX_debug_info {
  _LIBCUDACXX_EXPORTED_FROM_ABI string what() const;

  const char* __file_;
  int __line_;
  const char* __pred_;
  const char* __msg_;
};

std::string __LIBCUDACXX_debug_info::what() const {
  string msg = __file_;
  msg += ":" + std::to_string(__line_) + ": _LIBCUDACXX_ASSERT '";
  msg += __pred_;
  msg += "' failed. ";
  msg += __msg_;
  return msg;
}

_LIBCUDACXX_NORETURN _LIBCUDACXX_EXPORTED_FROM_ABI void __LIBCUDACXX_abort_debug_function(__LIBCUDACXX_debug_info const& info) {
  std::fprintf(stderr, "%s\n", info.what().c_str());
  std::abort();
}

typedef void (*__LIBCUDACXX_debug_function_type)(__LIBCUDACXX_debug_info const&);

_LIBCUDACXX_EXPORTED_FROM_ABI
constinit __LIBCUDACXX_debug_function_type __LIBCUDACXX_debug_function = __LIBCUDACXX_abort_debug_function;

_LIBCUDACXX_EXPORTED_FROM_ABI
bool __LIBCUDACXX_set_debug_function(__LIBCUDACXX_debug_function_type __func) {
  __LIBCUDACXX_debug_function = __func;
  return true;
}

_LIBCUDACXX_END_NAMESPACE_STD
