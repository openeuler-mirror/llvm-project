//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___CONCEPTS_CLASS_OR_ENUM_H
#define _LIBCUDACXX___CONCEPTS_CLASS_OR_ENUM_H

#include <__config>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

// Whether a type is a class type or enumeration type according to the Core wording.

template<class _Tp>
concept __class_or_enum = is_class_v<_Tp> || is_union_v<_Tp> || is_enum_v<_Tp>;

// Work around Clang bug https://llvm.org/PR52970
// TODO: remove this workaround once libc++ no longer has to support Clang 13 (it was fixed in Clang 14).
template<class _Tp>
concept __workaround_52970 = is_class_v<__uncvref_t<_Tp>> || is_union_v<__uncvref_t<_Tp>>;

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___CONCEPTS_CLASS_OR_ENUM_H
