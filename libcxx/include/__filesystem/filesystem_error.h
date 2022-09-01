// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FILESYSTEM_FILESYSTEM_ERROR_H
#define _LIBCUDACXX___FILESYSTEM_FILESYSTEM_ERROR_H

#include <__availability>
#include <__config>
#include <__filesystem/path.h>
#include <__memory/shared_ptr.h>
#include <iosfwd>
#include <new>
#include <system_error>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#ifndef _LIBCUDACXX_CXX03_LANG

_LIBCUDACXX_BEGIN_NAMESPACE_FILESYSTEM

class _LIBCUDACXX_AVAILABILITY_FILESYSTEM _LIBCUDACXX_EXCEPTION_ABI filesystem_error : public system_error {
public:
  _LIBCUDACXX_INLINE_VISIBILITY
  filesystem_error(const string& __what, error_code __ec)
      : system_error(__ec, __what),
        __storage_(make_shared<_Storage>(path(), path())) {
    __create_what(0);
  }

  _LIBCUDACXX_INLINE_VISIBILITY
  filesystem_error(const string& __what, const path& __p1, error_code __ec)
      : system_error(__ec, __what),
        __storage_(make_shared<_Storage>(__p1, path())) {
    __create_what(1);
  }

  _LIBCUDACXX_INLINE_VISIBILITY
  filesystem_error(const string& __what, const path& __p1, const path& __p2,
                   error_code __ec)
      : system_error(__ec, __what),
        __storage_(make_shared<_Storage>(__p1, __p2)) {
    __create_what(2);
  }

  _LIBCUDACXX_INLINE_VISIBILITY
  const path& path1() const noexcept { return __storage_->__p1_; }

  _LIBCUDACXX_INLINE_VISIBILITY
  const path& path2() const noexcept { return __storage_->__p2_; }

  filesystem_error(const filesystem_error&) = default;
  ~filesystem_error() override; // key function

  _LIBCUDACXX_INLINE_VISIBILITY
  const char* what() const noexcept override {
    return __storage_->__what_.c_str();
  }

  void __create_what(int __num_paths);

private:
  struct _LIBCUDACXX_HIDDEN _Storage {
    _LIBCUDACXX_INLINE_VISIBILITY
    _Storage(const path& __p1, const path& __p2) : __p1_(__p1), __p2_(__p2) {}

    path __p1_;
    path __p2_;
    string __what_;
  };
  shared_ptr<_Storage> __storage_;
};

// TODO(ldionne): We need to pop the pragma and push it again after
//                filesystem_error to work around PR41078.
_LIBCUDACXX_AVAILABILITY_FILESYSTEM_PUSH

template <class... _Args>
_LIBCUDACXX_NORETURN inline _LIBCUDACXX_INLINE_VISIBILITY
#ifndef _LIBCUDACXX_NO_EXCEPTIONS
void __throw_filesystem_error(_Args&&... __args) {
  throw filesystem_error(_CUDA_VSTD::forward<_Args>(__args)...);
}
#else
void __throw_filesystem_error(_Args&&...) {
  _CUDA_VSTD::abort();
}
#endif
_LIBCUDACXX_AVAILABILITY_FILESYSTEM_POP

_LIBCUDACXX_END_NAMESPACE_FILESYSTEM

#endif // _LIBCUDACXX_CXX03_LANG

#endif // _LIBCUDACXX___FILESYSTEM_FILESYSTEM_ERROR_H
