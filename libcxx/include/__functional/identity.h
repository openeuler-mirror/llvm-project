// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FUNCTIONAL_IDENTITY_H
#define _LIBCUDACXX___FUNCTIONAL_IDENTITY_H

#include <__config>
#include <__utility/forward.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

struct __identity {
  template <class _Tp>
  _LIBCUDACXX_NODISCARD _LIBCUDACXX_CONSTEXPR _Tp&& operator()(_Tp&& __t) const _NOEXCEPT {
    return std::forward<_Tp>(__t);
  }

  using is_transparent = void;
};

#if _LIBCUDACXX_STD_VER > 17

struct identity {
    template<class _Tp>
    _LIBCUDACXX_NODISCARD_EXT constexpr _Tp&& operator()(_Tp&& __t) const noexcept
    {
        return _CUDA_VSTD::forward<_Tp>(__t);
    }

    using is_transparent = void;
};
#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___FUNCTIONAL_IDENTITY_H
