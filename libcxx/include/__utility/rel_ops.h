//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___UTILITY_REL_OPS_H
#define _LIBCUDACXX___UTILITY_REL_OPS_H

#include <__config>
#include <__utility/forward.h>
#include <__utility/move.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

namespace rel_ops
{

template<class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY
bool
operator!=(const _Tp& __x, const _Tp& __y)
{
    return !(__x == __y);
}

template<class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY
bool
operator> (const _Tp& __x, const _Tp& __y)
{
    return __y < __x;
}

template<class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY
bool
operator<=(const _Tp& __x, const _Tp& __y)
{
    return !(__y < __x);
}

template<class _Tp>
inline _LIBCUDACXX_INLINE_VISIBILITY
bool
operator>=(const _Tp& __x, const _Tp& __y)
{
    return !(__x < __y);
}

} // namespace rel_ops

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___UTILITY_REL_OPS_H
