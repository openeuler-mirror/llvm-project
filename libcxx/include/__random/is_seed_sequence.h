//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___RANDOM_IS_SEED_SEQUENCE_H
#define _LIBCUDACXX___RANDOM_IS_SEED_SEQUENCE_H

#include <__config>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Sseq, class _Engine>
struct __is_seed_sequence
{
    static _LIBCUDACXX_CONSTEXPR const bool value =
              !is_convertible<_Sseq, typename _Engine::result_type>::value &&
              !is_same<typename remove_cv<_Sseq>::type, _Engine>::value;
};

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___RANDOM_IS_SEED_SEQUENCE_H
