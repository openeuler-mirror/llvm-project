// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___MEMORY_CONSTRUCT_AT_H
#define _LIBCUDACXX___MEMORY_CONSTRUCT_AT_H

#include <__assert>
#include <__config>
#include <__iterator/access.h>
#include <__memory/addressof.h>
#include <__memory/voidify.h>
#include <__utility/forward.h>
#include <__utility/move.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

// construct_at

#if _LIBCUDACXX_STD_VER > 17

template <class _Tp, class... _Args, class = decltype(::new(declval<void*>()) _Tp(declval<_Args>()...))>
_LIBCUDACXX_HIDE_FROM_ABI constexpr _Tp* construct_at(_Tp* __location, _Args&&... __args) {
  _LIBCUDACXX_ASSERT(__location != nullptr, "null pointer given to construct_at");
  return ::new (_CUDA_VSTD::__voidify(*__location)) _Tp(_CUDA_VSTD::forward<_Args>(__args)...);
}

#endif

template <class _Tp, class... _Args, class = decltype(::new(declval<void*>()) _Tp(declval<_Args>()...))>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR _Tp* __construct_at(_Tp* __location, _Args&&... __args) {
#if _LIBCUDACXX_STD_VER > 17
  return std::construct_at(__location, std::forward<_Args>(__args)...);
#else
  return _LIBCUDACXX_ASSERT(__location != nullptr, "null pointer given to construct_at"),
         ::new (std::__voidify(*__location)) _Tp(std::forward<_Args>(__args)...);
#endif
}

// destroy_at

// The internal functions are available regardless of the language version (with the exception of the `__destroy_at`
// taking an array).

template <class _ForwardIterator>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
_ForwardIterator __destroy(_ForwardIterator, _ForwardIterator);

template <class _Tp, typename enable_if<!is_array<_Tp>::value, int>::type = 0>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
void __destroy_at(_Tp* __loc) {
    _LIBCUDACXX_ASSERT(__loc != nullptr, "null pointer given to destroy_at");
    __loc->~_Tp();
}

#if _LIBCUDACXX_STD_VER > 17
template <class _Tp, typename enable_if<is_array<_Tp>::value, int>::type = 0>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
void __destroy_at(_Tp* __loc) {
    _LIBCUDACXX_ASSERT(__loc != nullptr, "null pointer given to destroy_at");
    _CUDA_VSTD::__destroy(_CUDA_VSTD::begin(*__loc), _CUDA_VSTD::end(*__loc));
}
#endif

template <class _ForwardIterator>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
_ForwardIterator __destroy(_ForwardIterator __first, _ForwardIterator __last) {
    for (; __first != __last; ++__first)
        _CUDA_VSTD::__destroy_at(_CUDA_VSTD::addressof(*__first));
    return __first;
}

#if _LIBCUDACXX_STD_VER > 14

template <class _Tp, enable_if_t<!is_array_v<_Tp>, int> = 0>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
void destroy_at(_Tp* __loc) {
    _CUDA_VSTD::__destroy_at(__loc);
}

#if _LIBCUDACXX_STD_VER > 17
template <class _Tp, enable_if_t<is_array_v<_Tp>, int> = 0>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
void destroy_at(_Tp* __loc) {
  _CUDA_VSTD::__destroy_at(__loc);
}
#endif

template <class _ForwardIterator>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
void destroy(_ForwardIterator __first, _ForwardIterator __last) {
  (void)_CUDA_VSTD::__destroy(_CUDA_VSTD::move(__first), _CUDA_VSTD::move(__last));
}

template <class _ForwardIterator, class _Size>
_LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
_ForwardIterator destroy_n(_ForwardIterator __first, _Size __n) {
    for (; __n > 0; (void)++__first, --__n)
        _CUDA_VSTD::__destroy_at(_CUDA_VSTD::addressof(*__first));
    return __first;
}

#endif // _LIBCUDACXX_STD_VER > 14

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___MEMORY_CONSTRUCT_AT_H
