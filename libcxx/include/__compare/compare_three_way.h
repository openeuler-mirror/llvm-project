// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___COMPARE_COMPARE_THREE_WAY_H
#define _LIBCUDACXX___COMPARE_COMPARE_THREE_WAY_H

#include <__compare/three_way_comparable.h>
#include <__config>
#include <__utility/forward.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

struct _LIBCUDACXX_TEMPLATE_VIS compare_three_way
{
    template<class _T1, class _T2>
        requires three_way_comparable_with<_T1, _T2>
    constexpr _LIBCUDACXX_HIDE_FROM_ABI
    auto operator()(_T1&& __t, _T2&& __u) const
        noexcept(noexcept(_CUDA_VSTD::forward<_T1>(__t) <=> _CUDA_VSTD::forward<_T2>(__u)))
        { return          _CUDA_VSTD::forward<_T1>(__t) <=> _CUDA_VSTD::forward<_T2>(__u); }

    using is_transparent = void;
};

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___COMPARE_COMPARE_THREE_WAY_H
