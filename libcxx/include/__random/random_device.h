//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___RANDOM_RANDOM_DEVICE_H
#define _LIBCUDACXX___RANDOM_RANDOM_DEVICE_H

#include <__config>
#include <string>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_PUSH_MACROS
#include <__undef_macros>

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if !defined(_LIBCUDACXX_HAS_NO_RANDOM_DEVICE)

class _LIBCUDACXX_TYPE_VIS random_device
{
#ifdef _LIBCUDACXX_USING_DEV_RANDOM
    int __f_;
#elif !defined(_LIBCUDACXX_ABI_NO_RANDOM_DEVICE_COMPATIBILITY_LAYOUT)
    _LIBCUDACXX_DIAGNOSTIC_PUSH
    _LIBCUDACXX_CLANG_DIAGNOSTIC_IGNORED("-Wunused-private-field")

    // Apple platforms used to use the `_LIBCUDACXX_USING_DEV_RANDOM` code path, and now
    // use `arc4random()` as of this comment. In order to avoid breaking the ABI, we
    // retain the same layout as before.
#   if defined(__APPLE__)
    int __padding_; // padding to fake the `__f_` field above
#   endif

    // ... vendors can add workarounds here if they switch to a different representation ...

    _LIBCUDACXX_DIAGNOSTIC_POP
#endif

public:
    // types
    typedef unsigned result_type;

    // generator characteristics
    static _LIBCUDACXX_CONSTEXPR const result_type _Min = 0;
    static _LIBCUDACXX_CONSTEXPR const result_type _Max = 0xFFFFFFFFu;

    _LIBCUDACXX_INLINE_VISIBILITY
    static _LIBCUDACXX_CONSTEXPR result_type min() { return _Min;}
    _LIBCUDACXX_INLINE_VISIBILITY
    static _LIBCUDACXX_CONSTEXPR result_type max() { return _Max;}

    // constructors
#ifndef _LIBCUDACXX_CXX03_LANG
    random_device() : random_device("/dev/urandom") {}
    explicit random_device(const string& __token);
#else
    explicit random_device(const string& __token = "/dev/urandom");
#endif
    ~random_device();

    // generating functions
    result_type operator()();

    // property functions
    double entropy() const _NOEXCEPT;

    random_device(const random_device&) = delete;
    void operator=(const random_device&) = delete;
};

#endif // !_LIBCUDACXX_HAS_NO_RANDOM_DEVICE

_LIBCUDACXX_END_NAMESPACE_STD

_LIBCUDACXX_POP_MACROS

#endif // _LIBCUDACXX___RANDOM_RANDOM_DEVICE_H
