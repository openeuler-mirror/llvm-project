//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___ALGORITHM_SHUFFLE_H
#define _LIBCUDACXX___ALGORITHM_SHUFFLE_H

#include <__algorithm/iterator_operations.h>
#include <__config>
#include <__debug>
#include <__iterator/iterator_traits.h>
#include <__random/uniform_int_distribution.h>
#include <__utility/forward.h>
#include <__utility/move.h>
#include <cstddef>
#include <cstdint>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_PUSH_MACROS
#include <__undef_macros>

_LIBCUDACXX_BEGIN_NAMESPACE_STD

class _LIBCUDACXX_TYPE_VIS __LIBCUDACXX_debug_randomizer {
public:
  __LIBCUDACXX_debug_randomizer() {
    __state = __seed();
    __inc = __state + 0xda3e39cb94b95bdbULL;
    __inc = (__inc << 1) | 1;
  }
  typedef uint_fast32_t result_type;

  static const result_type _Min = 0;
  static const result_type _Max = 0xFFFFFFFF;

  _LIBCUDACXX_HIDE_FROM_ABI result_type operator()() {
    uint_fast64_t __oldstate = __state;
    __state = __oldstate * 6364136223846793005ULL + __inc;
    return __oldstate >> 32;
  }

  static _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR result_type min() { return _Min; }
  static _LIBCUDACXX_HIDE_FROM_ABI _LIBCUDACXX_CONSTEXPR result_type max() { return _Max; }

private:
  uint_fast64_t __state;
  uint_fast64_t __inc;
  _LIBCUDACXX_HIDE_FROM_ABI static uint_fast64_t __seed() {
#ifdef _LIBCUDACXX_DEBUG_RANDOMIZE_UNSPECIFIED_STABILITY_SEED
    return _LIBCUDACXX_DEBUG_RANDOMIZE_UNSPECIFIED_STABILITY_SEED;
#else
    static char __x;
    return reinterpret_cast<uintptr_t>(&__x);
#endif
  }
};

#if _LIBCUDACXX_STD_VER <= 14 || defined(_LIBCUDACXX_ENABLE_CXX17_REMOVED_RANDOM_SHUFFLE) \
  || defined(_LIBCUDACXX_BUILDING_LIBRARY)
class _LIBCUDACXX_TYPE_VIS __rs_default;

_LIBCUDACXX_FUNC_VIS __rs_default __rs_get();

class _LIBCUDACXX_TYPE_VIS __rs_default
{
    static unsigned __c_;

    __rs_default();
public:
    typedef uint_fast32_t result_type;

    static const result_type _Min = 0;
    static const result_type _Max = 0xFFFFFFFF;

    __rs_default(const __rs_default&);
    ~__rs_default();

    result_type operator()();

    static _LIBCUDACXX_CONSTEXPR result_type min() {return _Min;}
    static _LIBCUDACXX_CONSTEXPR result_type max() {return _Max;}

    friend _LIBCUDACXX_FUNC_VIS __rs_default __rs_get();
};

_LIBCUDACXX_FUNC_VIS __rs_default __rs_get();

template <class _RandomAccessIterator>
_LIBCUDACXX_DEPRECATED_IN_CXX14 void
random_shuffle(_RandomAccessIterator __first, _RandomAccessIterator __last)
{
    typedef typename iterator_traits<_RandomAccessIterator>::difference_type difference_type;
    typedef uniform_int_distribution<ptrdiff_t> _Dp;
    typedef typename _Dp::param_type _Pp;
    difference_type __d = __last - __first;
    if (__d > 1)
    {
        _Dp __uid;
        __rs_default __g = __rs_get();
        for (--__last, (void) --__d; __first < __last; ++__first, (void) --__d)
        {
            difference_type __i = __uid(__g, _Pp(0, __d));
            if (__i != difference_type(0))
                swap(*__first, *(__first + __i));
        }
    }
}

template <class _RandomAccessIterator, class _RandomNumberGenerator>
_LIBCUDACXX_DEPRECATED_IN_CXX14 void
random_shuffle(_RandomAccessIterator __first, _RandomAccessIterator __last,
#ifndef _LIBCUDACXX_CXX03_LANG
               _RandomNumberGenerator&& __rand)
#else
               _RandomNumberGenerator& __rand)
#endif
{
    typedef typename iterator_traits<_RandomAccessIterator>::difference_type difference_type;
    difference_type __d = __last - __first;
    if (__d > 1)
    {
        for (--__last; __first < __last; ++__first, (void) --__d)
        {
            difference_type __i = __rand(__d);
            if (__i != difference_type(0))
              swap(*__first, *(__first + __i));
        }
    }
}
#endif

template <class _AlgPolicy, class _RandomAccessIterator, class _Sentinel, class _UniformRandomNumberGenerator>
_RandomAccessIterator __shuffle(
    _RandomAccessIterator __first, _Sentinel __last_sentinel, _UniformRandomNumberGenerator&& __g) {
    typedef typename iterator_traits<_RandomAccessIterator>::difference_type difference_type;
    typedef uniform_int_distribution<ptrdiff_t> _Dp;
    typedef typename _Dp::param_type _Pp;

    auto __original_last = _IterOps<_AlgPolicy>::next(__first, __last_sentinel);
    auto __last = __original_last;
    difference_type __d = __last - __first;
    if (__d > 1)
    {
        _Dp __uid;
        for (--__last, (void) --__d; __first < __last; ++__first, (void) --__d)
        {
            difference_type __i = __uid(__g, _Pp(0, __d));
            if (__i != difference_type(0))
                _IterOps<_AlgPolicy>::iter_swap(__first, __first + __i);
        }
    }

    return __original_last;
}

template <class _RandomAccessIterator, class _UniformRandomNumberGenerator>
void shuffle(_RandomAccessIterator __first, _RandomAccessIterator __last,
             _UniformRandomNumberGenerator&& __g) {
  (void)std::__shuffle<_ClassicAlgPolicy>(
      std::move(__first), std::move(__last), std::forward<_UniformRandomNumberGenerator>(__g));
}

_LIBCUDACXX_END_NAMESPACE_STD

_LIBCUDACXX_POP_MACROS

#endif // _LIBCUDACXX___ALGORITHM_SHUFFLE_H
