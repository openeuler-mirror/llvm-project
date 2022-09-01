// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___MEMORY_ADDRESSOF_H
#define _LIBCUDACXX___MEMORY_ADDRESSOF_H

#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp>
inline _LIBCUDACXX_CONSTEXPR_AFTER_CXX14
_LIBCUDACXX_NO_CFI _LIBCUDACXX_INLINE_VISIBILITY
_Tp*
addressof(_Tp& __x) _NOEXCEPT
{
    return __builtin_addressof(__x);
}

#if defined(_LIBCUDACXX_HAS_OBJC_ARC) && !defined(_LIBCUDACXX_PREDEFINED_OBJC_ARC_ADDRESSOF)
// Objective-C++ Automatic Reference Counting uses qualified pointers
// that require special addressof() signatures. When
// _LIBCUDACXX_PREDEFINED_OBJC_ARC_ADDRESSOF is defined, the compiler
// itself is providing these definitions. Otherwise, we provide them.
template <class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY
__strong _Tp*
addressof(__strong _Tp& __x) _NOEXCEPT
{
  return &__x;
}

#ifdef _LIBCUDACXX_HAS_OBJC_ARC_WEAK
template <class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY
__weak _Tp*
addressof(__weak _Tp& __x) _NOEXCEPT
{
  return &__x;
}
#endif

template <class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY
__autoreleasing _Tp*
addressof(__autoreleasing _Tp& __x) _NOEXCEPT
{
  return &__x;
}

template <class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY
__unsafe_unretained _Tp*
addressof(__unsafe_unretained _Tp& __x) _NOEXCEPT
{
  return &__x;
}
#endif

#if !defined(_LIBCUDACXX_CXX03_LANG)
template <class _Tp> _Tp* addressof(const _Tp&&) noexcept = delete;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___MEMORY_ADDRESSOF_H
