//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___TYPE_TRAITS_APPLY_CV_H
#define _LIBCUDACXX___TYPE_TRAITS_APPLY_CV_H

#include <__config>
#include <__type_traits/integral_constant.h>
#include <__type_traits/is_const.h>
#include <__type_traits/is_volatile.h>
#include <__type_traits/remove_reference.h>
#include <cstddef>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

template <class _Tp, class _Up, bool = is_const<typename remove_reference<_Tp>::type>::value,
                             bool = is_volatile<typename remove_reference<_Tp>::type>::value>
struct __apply_cv
{
    typedef _LIBCUDACXX_NODEBUG _Up type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp, _Up, true, false>
{
    typedef _LIBCUDACXX_NODEBUG const _Up type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp, _Up, false, true>
{
    typedef volatile _Up type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp, _Up, true, true>
{
    typedef const volatile _Up type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp&, _Up, false, false>
{
    typedef _Up& type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp&, _Up, true, false>
{
    typedef const _Up& type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp&, _Up, false, true>
{
    typedef volatile _Up& type;
};

template <class _Tp, class _Up>
struct __apply_cv<_Tp&, _Up, true, true>
{
    typedef const volatile _Up& type;
};

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___TYPE_TRAITS_APPLY_CV_H
