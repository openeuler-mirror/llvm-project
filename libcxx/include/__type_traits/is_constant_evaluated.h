//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_IS_CONSTANT_EVALUATED_H
#define _LIBCUDACXX___TYPE_TRAITS_IS_CONSTANT_EVALUATED_H

#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17
_LIBCUDACXX_INLINE_VISIBILITY
inline constexpr bool is_constant_evaluated() noexcept {
  return __builtin_is_constant_evaluated();
}
#endif

inline _LIBCUDACXX_CONSTEXPR
bool __LIBCUDACXX_is_constant_evaluated() _NOEXCEPT { return __builtin_is_constant_evaluated(); }

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_IS_CONSTANT_EVALUATED_H
