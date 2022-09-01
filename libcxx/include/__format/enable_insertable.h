// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FORMAT_ENABLE_INSERTABLE_H
#define _LIBCUDACXX___FORMAT_ENABLE_INSERTABLE_H

#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

namespace __format {

/// Opt-in to enable \ref __insertable for a \a _Container.
template <class _Container>
inline constexpr bool __enable_insertable = false;

} // namespace __format

#endif //_LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___FORMAT_ENABLE_INSERTABLE_H
