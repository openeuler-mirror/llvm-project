//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef __LIBCUDACXX___COROUTINE_TRIVIAL_AWAITABLES_H
#define __LIBCUDACXX___COROUTINE_TRIVIAL_AWAITABLES_H

#include <__config>
#include <__coroutine/coroutine_handle.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCUDACXX_STD_VER > 17 && !defined(_LIBCUDACXX_HAS_NO_CXX20_COROUTINES)

_LIBCUDACXX_BEGIN_NAMESPACE_STD

// [coroutine.trivial.awaitables]
struct suspend_never {
    _LIBCUDACXX_HIDE_FROM_ABI
    constexpr bool await_ready() const noexcept { return true; }
    _LIBCUDACXX_HIDE_FROM_ABI
    constexpr void await_suspend(coroutine_handle<>) const noexcept {}
    _LIBCUDACXX_HIDE_FROM_ABI
    constexpr void await_resume() const noexcept {}
};

struct suspend_always {
    _LIBCUDACXX_HIDE_FROM_ABI
    constexpr bool await_ready() const noexcept { return false; }
    _LIBCUDACXX_HIDE_FROM_ABI
    constexpr void await_suspend(coroutine_handle<>) const noexcept {}
    _LIBCUDACXX_HIDE_FROM_ABI
    constexpr void await_resume() const noexcept {}
};

_LIBCUDACXX_END_NAMESPACE_STD

#endif // __LIBCUDACXX_STD_VER > 17 && !defined(_LIBCUDACXX_HAS_NO_CXX20_COROUTINES)

#endif // __LIBCUDACXX___COROUTINE_TRIVIAL_AWAITABLES_H
