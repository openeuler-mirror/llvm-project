//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_RANGES_UNIFORM_RANDOM_BIT_GENERATOR_ADAPTOR_H
#define _LIBCUDACXX___ALGORITHM_RANGES_UNIFORM_RANDOM_BIT_GENERATOR_ADAPTOR_H

#include <__config>
#include <__functional/invoke.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_PUSH_MACROS
#include <__undef_macros>

_LIBCUDACXX_BEGIN_NAMESPACE_STD

// Range versions of random algorithms (e.g. `std::shuffle`) are less constrained than their classic counterparts.
// Range algorithms only require the given generator to satisfy the `std::uniform_random_bit_generator` concept.
// Classic algorithms require the given generator to meet the uniform random bit generator requirements; these
// requirements include satisfying `std::uniform_random_bit_generator` and add a requirement for the generator to
// provide a nested `result_type` typedef (see `[rand.req.urng]`).
//
// To be able to reuse classic implementations, make the given generator meet the classic requirements by wrapping
// it into an adaptor type that forwards all of its interface and adds the required typedef.
template <class _Gen>
class _ClassicGenAdaptor {
private:
  // The generator is not required to be copyable or movable, so it has to be stored as a reference.
  _Gen& __gen;

public:
  using result_type = invoke_result_t<_Gen&>;

  _LIBCUDACXX_HIDE_FROM_ABI
  static constexpr auto min() { return __uncvref_t<_Gen>::min(); }
  _LIBCUDACXX_HIDE_FROM_ABI
  static constexpr auto max() { return __uncvref_t<_Gen>::max(); }

  _LIBCUDACXX_HIDE_FROM_ABI
  constexpr explicit _ClassicGenAdaptor(_Gen& __g) : __gen(__g) {}

  _LIBCUDACXX_HIDE_FROM_ABI
  constexpr auto operator()() const { return __gen(); }
};

_LIBCUDACXX_END_NAMESPACE_STD

_LIBCUDACXX_POP_MACROS

#endif // _LIBCUDACXX_STD_VER > 17

#endif // _LIBCUDACXX___ALGORITHM_RANGES_UNIFORM_RANDOM_BIT_GENERATOR_ADAPTOR_H
