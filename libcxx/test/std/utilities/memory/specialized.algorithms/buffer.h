// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LIBCUDACXX_TEST_STD_UTILITIES_MEMORY_SPECIALIZED_ALGORITHMS_BUFFER_H
#define LIBCUDACXX_TEST_STD_UTILITIES_MEMORY_SPECIALIZED_ALGORITHMS_BUFFER_H

template <typename T, int N>
struct Buffer {
  alignas(T) char buffer[sizeof(T) * N] = {};

  T* begin() { return reinterpret_cast<T*>(buffer); }
  T* end() { return begin() + N; }
  const T* cbegin() const { return reinterpret_cast<const T*>(buffer); }
  const T* cend() const { return cbegin() + N; }

  constexpr int size() const { return N; }
};

#endif // LIBCUDACXX_TEST_STD_UTILITIES_MEMORY_SPECIALIZED_ALGORITHMS_BUFFER_H
