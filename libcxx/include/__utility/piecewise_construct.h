//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___UTILITY_PIECEWISE_CONSTRUCT_H
#define _LIBCUDACXX___UTILITY_PIECEWISE_CONSTRUCT_H

#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

struct _LIBCUDACXX_TEMPLATE_VIS piecewise_construct_t { explicit piecewise_construct_t() = default; };
#if defined(_LIBCUDACXX_CXX03_LANG) || defined(_LIBCUDACXX_BUILDING_LIBRARY)
extern _LIBCUDACXX_EXPORTED_FROM_ABI const piecewise_construct_t piecewise_construct;// = piecewise_construct_t();
#else
/* inline */ constexpr piecewise_construct_t piecewise_construct = piecewise_construct_t();
#endif

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___UTILITY_PIECEWISE_CONSTRUCT_H
