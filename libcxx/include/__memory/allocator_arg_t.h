// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FUNCTIONAL_ALLOCATOR_ARG_T_H
#define _LIBCUDACXX___FUNCTIONAL_ALLOCATOR_ARG_T_H

#include <__config>
#include <__memory/uses_allocator.h>
#include <__utility/forward.h>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

struct _LIBCUDACXX_TEMPLATE_VIS allocator_arg_t { explicit allocator_arg_t() = default; };

#if defined(_LIBCUDACXX_CXX03_LANG) || defined(_LIBCUDACXX_BUILDING_LIBRARY)
extern _LIBCUDACXX_EXPORTED_FROM_ABI const allocator_arg_t allocator_arg;
#else
/* inline */ constexpr allocator_arg_t allocator_arg = allocator_arg_t();
#endif

#ifndef _LIBCUDACXX_CXX03_LANG

// allocator construction

template <class _Tp, class _Alloc, class ..._Args>
struct __uses_alloc_ctor_imp
{
    typedef _LIBCUDACXX_NODEBUG __uncvref_t<_Alloc> _RawAlloc;
    static const bool __ua = uses_allocator<_Tp, _RawAlloc>::value;
    static const bool __ic =
        is_constructible<_Tp, allocator_arg_t, _Alloc, _Args...>::value;
    static const int value = __ua ? 2 - __ic : 0;
};

template <class _Tp, class _Alloc, class ..._Args>
struct __uses_alloc_ctor
    : integral_constant<int, __uses_alloc_ctor_imp<_Tp, _Alloc, _Args...>::value>
    {};

template <class _Tp, class _Allocator, class... _Args>
inline _LIBCUDACXX_INLINE_VISIBILITY
void __user_alloc_construct_impl (integral_constant<int, 0>, _Tp *__storage, const _Allocator &, _Args &&... __args )
{
    new (__storage) _Tp (_CUDA_VSTD::forward<_Args>(__args)...);
}

// FIXME: This should have a version which takes a non-const alloc.
template <class _Tp, class _Allocator, class... _Args>
inline _LIBCUDACXX_INLINE_VISIBILITY
void __user_alloc_construct_impl (integral_constant<int, 1>, _Tp *__storage, const _Allocator &__a, _Args &&... __args )
{
    new (__storage) _Tp (allocator_arg, __a, _CUDA_VSTD::forward<_Args>(__args)...);
}

// FIXME: This should have a version which takes a non-const alloc.
template <class _Tp, class _Allocator, class... _Args>
inline _LIBCUDACXX_INLINE_VISIBILITY
void __user_alloc_construct_impl (integral_constant<int, 2>, _Tp *__storage, const _Allocator &__a, _Args &&... __args )
{
    new (__storage) _Tp (_CUDA_VSTD::forward<_Args>(__args)..., __a);
}

#endif // _LIBCUDACXX_CXX03_LANG

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___FUNCTIONAL_ALLOCATOR_ARG_T_H
