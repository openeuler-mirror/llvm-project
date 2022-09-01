//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <vector>

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#ifndef _LIBCUDACXX_ABI_DO_NOT_EXPORT_VECTOR_BASE_COMMON

template <bool>
struct __vector_base_common;

template <>
struct __vector_base_common<true> {
  _LIBCUDACXX_NORETURN _LIBCUDACXX_EXPORTED_FROM_ABI void __throw_length_error() const;
  _LIBCUDACXX_NORETURN _LIBCUDACXX_EXPORTED_FROM_ABI void __throw_out_of_range() const;
};

void __vector_base_common<true>::__throw_length_error() const {
  _CUDA_VSTD::__throw_length_error("vector");
}

void __vector_base_common<true>::__throw_out_of_range() const {
  _CUDA_VSTD::__throw_out_of_range("vector");
}

#endif // _LIBCUDACXX_ABI_DO_NOT_EXPORT_VECTOR_BASE_COMMON

_LIBCUDACXX_END_NAMESPACE_STD
