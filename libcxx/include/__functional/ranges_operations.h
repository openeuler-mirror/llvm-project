// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FUNCTIONAL_RANGES_OPERATIONS_H
#define _LIBCUDACXX___FUNCTIONAL_RANGES_OPERATIONS_H

#include <__config>
#include <__utility/forward.h>
#include <concepts>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

namespace ranges {

struct equal_to {
  template <class _Tp, class _Up>
  requires equality_comparable_with<_Tp, _Up>
  [[nodiscard]] constexpr bool operator()(_Tp &&__t, _Up &&__u) const
      noexcept(noexcept(bool(_CUDA_VSTD::forward<_Tp>(__t) == _CUDA_VSTD::forward<_Up>(__u)))) {
    return _CUDA_VSTD::forward<_Tp>(__t) == _CUDA_VSTD::forward<_Up>(__u);
  }

  using is_transparent = void;
};

struct not_equal_to {
  template <class _Tp, class _Up>
  requires equality_comparable_with<_Tp, _Up>
  [[nodiscard]] constexpr bool operator()(_Tp &&__t, _Up &&__u) const
      noexcept(noexcept(bool(!(_CUDA_VSTD::forward<_Tp>(__t) == _CUDA_VSTD::forward<_Up>(__u))))) {
    return !(_CUDA_VSTD::forward<_Tp>(__t) == _CUDA_VSTD::forward<_Up>(__u));
  }

  using is_transparent = void;
};

struct less {
  template <class _Tp, class _Up>
  requires totally_ordered_with<_Tp, _Up>
  [[nodiscard]] constexpr bool operator()(_Tp &&__t, _Up &&__u) const
      noexcept(noexcept(bool(_CUDA_VSTD::forward<_Tp>(__t) < _CUDA_VSTD::forward<_Up>(__u)))) {
    return _CUDA_VSTD::forward<_Tp>(__t) < _CUDA_VSTD::forward<_Up>(__u);
  }

  using is_transparent = void;
};

struct less_equal {
  template <class _Tp, class _Up>
  requires totally_ordered_with<_Tp, _Up>
  [[nodiscard]] constexpr bool operator()(_Tp &&__t, _Up &&__u) const
      noexcept(noexcept(bool(!(_CUDA_VSTD::forward<_Up>(__u) < _CUDA_VSTD::forward<_Tp>(__t))))) {
    return !(_CUDA_VSTD::forward<_Up>(__u) < _CUDA_VSTD::forward<_Tp>(__t));
  }

  using is_transparent = void;
};

struct greater {
  template <class _Tp, class _Up>
  requires totally_ordered_with<_Tp, _Up>
  [[nodiscard]] constexpr bool operator()(_Tp &&__t, _Up &&__u) const
      noexcept(noexcept(bool(_CUDA_VSTD::forward<_Up>(__u) < _CUDA_VSTD::forward<_Tp>(__t)))) {
    return _CUDA_VSTD::forward<_Up>(__u) < _CUDA_VSTD::forward<_Tp>(__t);
  }

  using is_transparent = void;
};

struct greater_equal {
  template <class _Tp, class _Up>
  requires totally_ordered_with<_Tp, _Up>
  [[nodiscard]] constexpr bool operator()(_Tp &&__t, _Up &&__u) const
      noexcept(noexcept(bool(!(_CUDA_VSTD::forward<_Tp>(__t) < _CUDA_VSTD::forward<_Up>(__u))))) {
    return !(_CUDA_VSTD::forward<_Tp>(__t) < _CUDA_VSTD::forward<_Up>(__u));
  }

  using is_transparent = void;
};

} // namespace ranges

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___FUNCTIONAL_RANGES_OPERATIONS_H
