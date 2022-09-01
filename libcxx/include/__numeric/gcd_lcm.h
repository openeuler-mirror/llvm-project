// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___NUMERIC_GCD_LCM_H
#define _LIBCUDACXX___NUMERIC_GCD_LCM_H

#include <__assert>
#include <__config>
#include <limits>
#include <type_traits>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_PUSH_MACROS
#include <__undef_macros>

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 14

template <typename _Result, typename _Source, bool _IsSigned = is_signed<_Source>::value> struct __ct_abs;

template <typename _Result, typename _Source>
struct __ct_abs<_Result, _Source, true> {
    _LIBCUDACXX_CONSTEXPR _LIBCUDACXX_INLINE_VISIBILITY
    _Result operator()(_Source __t) const noexcept
    {
        if (__t >= 0) return __t;
        if (__t == numeric_limits<_Source>::min()) return -static_cast<_Result>(__t);
        return -__t;
    }
};

template <typename _Result, typename _Source>
struct __ct_abs<_Result, _Source, false> {
    _LIBCUDACXX_CONSTEXPR _LIBCUDACXX_INLINE_VISIBILITY
    _Result operator()(_Source __t) const noexcept { return __t; }
};


template<class _Tp>
_LIBCUDACXX_CONSTEXPR _LIBCUDACXX_HIDDEN
_Tp __gcd(_Tp __m, _Tp __n)
{
    static_assert((!is_signed<_Tp>::value), "");
    return __n == 0 ? __m : _CUDA_VSTD::__gcd<_Tp>(__n, __m % __n);
}

template<class _Tp, class _Up>
_LIBCUDACXX_CONSTEXPR _LIBCUDACXX_INLINE_VISIBILITY
common_type_t<_Tp,_Up>
gcd(_Tp __m, _Up __n)
{
    static_assert((is_integral<_Tp>::value && is_integral<_Up>::value), "Arguments to gcd must be integer types");
    static_assert((!is_same<typename remove_cv<_Tp>::type, bool>::value), "First argument to gcd cannot be bool" );
    static_assert((!is_same<typename remove_cv<_Up>::type, bool>::value), "Second argument to gcd cannot be bool" );
    using _Rp = common_type_t<_Tp,_Up>;
    using _Wp = make_unsigned_t<_Rp>;
    return static_cast<_Rp>(_CUDA_VSTD::__gcd(
        static_cast<_Wp>(__ct_abs<_Rp, _Tp>()(__m)),
        static_cast<_Wp>(__ct_abs<_Rp, _Up>()(__n))));
}

template<class _Tp, class _Up>
_LIBCUDACXX_CONSTEXPR _LIBCUDACXX_INLINE_VISIBILITY
common_type_t<_Tp,_Up>
lcm(_Tp __m, _Up __n)
{
    static_assert((is_integral<_Tp>::value && is_integral<_Up>::value), "Arguments to lcm must be integer types");
    static_assert((!is_same<typename remove_cv<_Tp>::type, bool>::value), "First argument to lcm cannot be bool" );
    static_assert((!is_same<typename remove_cv<_Up>::type, bool>::value), "Second argument to lcm cannot be bool" );
    if (__m == 0 || __n == 0)
        return 0;

    using _Rp = common_type_t<_Tp,_Up>;
    _Rp __val1 = __ct_abs<_Rp, _Tp>()(__m) / _CUDA_VSTD::gcd(__m, __n);
    _Rp __val2 = __ct_abs<_Rp, _Up>()(__n);
    _LIBCUDACXX_ASSERT((numeric_limits<_Rp>::max() / __val1 > __val2), "Overflow in lcm");
    return __val1 * __val2;
}

#endif // _LIBCUDACXX_STD_VER

_LIBCUDACXX_END_NAMESPACE_STD

_LIBCUDACXX_POP_MACROS

#endif // _LIBCUDACXX___NUMERIC_GCD_LCM_H
