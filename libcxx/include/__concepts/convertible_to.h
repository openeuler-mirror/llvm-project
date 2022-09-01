//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___CONCEPTS_CONVERTIBLE_TO_H
#define _LIBCUDACXX___CONCEPTS_CONVERTIBLE_TO_H

#include <__config>
#include <__utility/declval.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

// [concept.convertible]

template<class _From, class _To>
concept convertible_to =
  is_convertible_v<_From, _To> &&
  requires {
    static_cast<_To>(declval<_From>());
  };

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___CONCEPTS_CONVERTIBLE_TO_H
