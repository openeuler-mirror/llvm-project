//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___UTILITY_AS_CONST_H
#define _LIBCUDACXX___UTILITY_AS_CONST_H

#include <__config>
#include <__utility/forward.h>
#include <__utility/move.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp>
_LIBCUDACXX_NODISCARD_EXT constexpr add_const_t<_Tp>& as_const(_Tp& __t) noexcept { return __t; }

template <class _Tp>
void as_const(const _Tp&&) = delete;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___UTILITY_AS_CONST_H
