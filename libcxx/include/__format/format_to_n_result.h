// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FORMAT_FORMAT_TO_N_RESULT_H
#define _LIBCUDACXX___FORMAT_FORMAT_TO_N_RESULT_H

#include <__config>
#include <__iterator/incrementable_traits.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

template <class _OutIt>
struct _LIBCUDACXX_TEMPLATE_VIS format_to_n_result {
  _OutIt out;
  iter_difference_t<_OutIt> size;
};

#endif //_LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___FORMAT_FORMAT_TO_N_RESULT_H
