//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_ALIGNMENT_OF_H
#define _LIBCUDACXX___TYPE_TRAITS_ALIGNMENT_OF_H

#include <__config>
#include <__type_traits/integral_constant.h>
#include <cstddef>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp> struct _LIBCUDACXX_TEMPLATE_VIS alignment_of
    : public integral_constant<size_t, _LIBCUDACXX_ALIGNOF(_Tp)> {};

#if _LIBCUDACXX_STD_VER > 14
template <class _Tp>
inline constexpr size_t alignment_of_v = alignment_of<_Tp>::value;
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_ALIGNMENT_OF_H
