//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___COMPARE_COMPARE_STRONG_ORDER_FALLBACK
#define _LIBCUDACXX___COMPARE_COMPARE_STRONG_ORDER_FALLBACK

#include <__compare/ordering.h>
#include <__compare/strong_order.h>
#include <__config>
#include <__utility/forward.h>
#include <__utility/priority_tag.h>
#include <type_traits>

#ifndef _LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

// [cmp.alg]
namespace __compare_strong_order_fallback {
    struct __fn {
        template<class _Tp, class _Up>
            requires is_same_v<decay_t<_Tp>, decay_t<_Up>>
        _LIBCUDACXX_HIDE_FROM_ABI static constexpr auto
        __go(_Tp&& __t, _Up&& __u, __priority_tag<1>)
            noexcept(noexcept(_CUDA_VSTD::strong_order(_CUDA_VSTD::forward<_Tp>(__t), _CUDA_VSTD::forward<_Up>(__u))))
            -> decltype(      _CUDA_VSTD::strong_order(_CUDA_VSTD::forward<_Tp>(__t), _CUDA_VSTD::forward<_Up>(__u)))
            { return          _CUDA_VSTD::strong_order(_CUDA_VSTD::forward<_Tp>(__t), _CUDA_VSTD::forward<_Up>(__u)); }

        template<class _Tp, class _Up>
            requires is_same_v<decay_t<_Tp>, decay_t<_Up>>
        _LIBCUDACXX_HIDE_FROM_ABI static constexpr auto
        __go(_Tp&& __t, _Up&& __u, __priority_tag<0>)
            noexcept(noexcept(_CUDA_VSTD::forward<_Tp>(__t) == _CUDA_VSTD::forward<_Up>(__u) ? strong_ordering::equal :
                              _CUDA_VSTD::forward<_Tp>(__t) < _CUDA_VSTD::forward<_Up>(__u) ? strong_ordering::less :
                              strong_ordering::greater))
            -> decltype(      _CUDA_VSTD::forward<_Tp>(__t) == _CUDA_VSTD::forward<_Up>(__u) ? strong_ordering::equal :
                              _CUDA_VSTD::forward<_Tp>(__t) < _CUDA_VSTD::forward<_Up>(__u) ? strong_ordering::less :
                              strong_ordering::greater)
        {
            return            _CUDA_VSTD::forward<_Tp>(__t) == _CUDA_VSTD::forward<_Up>(__u) ? strong_ordering::equal :
                              _CUDA_VSTD::forward<_Tp>(__t) < _CUDA_VSTD::forward<_Up>(__u) ? strong_ordering::less :
                              strong_ordering::greater;
        }

        template<class _Tp, class _Up>
        _LIBCUDACXX_HIDE_FROM_ABI constexpr auto operator()(_Tp&& __t, _Up&& __u) const
            noexcept(noexcept(__go(_CUDA_VSTD::forward<_Tp>(__t), _CUDA_VSTD::forward<_Up>(__u), __priority_tag<1>())))
            -> decltype(      __go(_CUDA_VSTD::forward<_Tp>(__t), _CUDA_VSTD::forward<_Up>(__u), __priority_tag<1>()))
            { return          __go(_CUDA_VSTD::forward<_Tp>(__t), _CUDA_VSTD::forward<_Up>(__u), __priority_tag<1>()); }
    };
} // namespace __compare_strong_order_fallback

inline namespace __cpo {
    inline constexpr auto compare_strong_order_fallback = __compare_strong_order_fallback::__fn{};
} // namespace __cpo

#endif // _LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___COMPARE_COMPARE_STRONG_ORDER_FALLBACK
