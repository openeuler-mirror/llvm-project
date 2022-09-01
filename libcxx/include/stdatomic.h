// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX_STDATOMIC_H
#define _LIBCUDACXX_STDATOMIC_H

/*
    stdatomic.h synopsis

template<class T>
  using std-atomic = std::atomic<T>;        // exposition only

#define _Atomic(T) std-atomic<T>

#define ATOMIC_BOOL_LOCK_FREE see below
#define ATOMIC_CHAR_LOCK_FREE see below
#define ATOMIC_CHAR16_T_LOCK_FREE see below
#define ATOMIC_CHAR32_T_LOCK_FREE see below
#define ATOMIC_WCHAR_T_LOCK_FREE see below
#define ATOMIC_SHORT_LOCK_FREE see below
#define ATOMIC_INT_LOCK_FREE see below
#define ATOMIC_LONG_LOCK_FREE see below
#define ATOMIC_LLONG_LOCK_FREE see below
#define ATOMIC_POINTER_LOCK_FREE see below

using std::memory_order                // see below
using std::memory_order_relaxed        // see below
using std::memory_order_consume        // see below
using std::memory_order_acquire        // see below
using std::memory_order_release        // see below
using std::memory_order_acq_rel        // see below
using std::memory_order_seq_cst        // see below

using std::atomic_flag                 // see below

using std::atomic_bool                 // see below
using std::atomic_char                 // see below
using std::atomic_schar                // see below
using std::atomic_uchar                // see below
using std::atomic_short                // see below
using std::atomic_ushort               // see below
using std::atomic_int                  // see below
using std::atomic_uint                 // see below
using std::atomic_long                 // see below
using std::atomic_ulong                // see below
using std::atomic_llong                // see below
using std::atomic_ullong               // see below
using std::atomic_char8_t              // see below
using std::atomic_char16_t             // see below
using std::atomic_char32_t             // see below
using std::atomic_wchar_t              // see below
using std::atomic_int8_t               // see below
using std::atomic_uint8_t              // see below
using std::atomic_int16_t              // see below
using std::atomic_uint16_t             // see below
using std::atomic_int32_t              // see below
using std::atomic_uint32_t             // see below
using std::atomic_int64_t              // see below
using std::atomic_uint64_t             // see below
using std::atomic_int_least8_t         // see below
using std::atomic_uint_least8_t        // see below
using std::atomic_int_least16_t        // see below
using std::atomic_uint_least16_t       // see below
using std::atomic_int_least32_t        // see below
using std::atomic_uint_least32_t       // see below
using std::atomic_int_least64_t        // see below
using std::atomic_uint_least64_t       // see below
using std::atomic_int_fast8_t          // see below
using std::atomic_uint_fast8_t         // see below
using std::atomic_int_fast16_t         // see below
using std::atomic_uint_fast16_t        // see below
using std::atomic_int_fast32_t         // see below
using std::atomic_uint_fast32_t        // see below
using std::atomic_int_fast64_t         // see below
using std::atomic_uint_fast64_t        // see below
using std::atomic_intptr_t             // see below
using std::atomic_uintptr_t            // see below
using std::atomic_size_t               // see below
using std::atomic_ptrdiff_t            // see below
using std::atomic_intmax_t             // see below
using std::atomic_uintmax_t            // see below

using std::atomic_is_lock_free                         // see below
using std::atomic_load                                 // see below
using std::atomic_load_explicit                        // see below
using std::atomic_store                                // see below
using std::atomic_store_explicit                       // see below
using std::atomic_exchange                             // see below
using std::atomic_exchange_explicit                    // see below
using std::atomic_compare_exchange_strong              // see below
using std::atomic_compare_exchange_strong_explicit     // see below
using std::atomic_compare_exchange_weak                // see below
using std::atomic_compare_exchange_weak_explicit       // see below
using std::atomic_fetch_add                            // see below
using std::atomic_fetch_add_explicit                   // see below
using std::atomic_fetch_sub                            // see below
using std::atomic_fetch_sub_explicit                   // see below
using std::atomic_fetch_or                             // see below
using std::atomic_fetch_or_explicit                    // see below
using std::atomic_fetch_and                            // see below
using std::atomic_fetch_and_explicit                   // see below
using std::atomic_flag_test_and_set                    // see below
using std::atomic_flag_test_and_set_explicit           // see below
using std::atomic_flag_clear                           // see below
using std::atomic_flag_clear_explicit                  // see below

using std::atomic_thread_fence                         // see below
using std::atomic_signal_fence                         // see below

*/

#include <__config>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

#if _LIBCUDACXX_STD_VER > 20

#include <atomic>
#include <version>

#ifdef _Atomic
# undef _Atomic
#endif

#define _Atomic(_Tp) ::std::atomic<_Tp>

using std::memory_order _LIBCUDACXX_USING_IF_EXISTS;
using std::memory_order_relaxed _LIBCUDACXX_USING_IF_EXISTS;
using std::memory_order_consume _LIBCUDACXX_USING_IF_EXISTS;
using std::memory_order_acquire _LIBCUDACXX_USING_IF_EXISTS;
using std::memory_order_release _LIBCUDACXX_USING_IF_EXISTS;
using std::memory_order_acq_rel _LIBCUDACXX_USING_IF_EXISTS;
using std::memory_order_seq_cst _LIBCUDACXX_USING_IF_EXISTS;

using std::atomic_flag _LIBCUDACXX_USING_IF_EXISTS;

using std::atomic_bool _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_char _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_schar _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uchar _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_short _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_ushort _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_int _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_long _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_ulong _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_llong _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_ullong _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_char8_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_char16_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_char32_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_wchar_t _LIBCUDACXX_USING_IF_EXISTS;

using std::atomic_int8_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint8_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_int16_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint16_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_int32_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint32_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_int64_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint64_t _LIBCUDACXX_USING_IF_EXISTS;

using std::atomic_int_least8_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint_least8_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_int_least16_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint_least16_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_int_least32_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint_least32_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_int_least64_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint_least64_t _LIBCUDACXX_USING_IF_EXISTS;

using std::atomic_int_fast8_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint_fast8_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_int_fast16_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint_fast16_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_int_fast32_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint_fast32_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_int_fast64_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uint_fast64_t _LIBCUDACXX_USING_IF_EXISTS;

using std::atomic_intptr_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uintptr_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_size_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_ptrdiff_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_intmax_t _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_uintmax_t _LIBCUDACXX_USING_IF_EXISTS;

using std::atomic_compare_exchange_strong _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_compare_exchange_strong_explicit _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_compare_exchange_weak _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_compare_exchange_weak_explicit _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_exchange _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_exchange_explicit _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_fetch_add _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_fetch_add_explicit _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_fetch_and _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_fetch_and_explicit _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_fetch_or _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_fetch_or_explicit _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_fetch_sub _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_fetch_sub_explicit _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_flag_clear _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_flag_clear_explicit _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_flag_test_and_set _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_flag_test_and_set_explicit _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_is_lock_free _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_load _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_load_explicit _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_store _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_store_explicit _LIBCUDACXX_USING_IF_EXISTS;

using std::atomic_signal_fence _LIBCUDACXX_USING_IF_EXISTS;
using std::atomic_thread_fence _LIBCUDACXX_USING_IF_EXISTS;

#elif defined(_LIBCUDACXX_COMPILER_CLANG_BASED)

// Before C++23, we include the next <stdatomic.h> on the path to avoid hijacking
// the header. We do this because Clang has historically shipped a <stdatomic.h>
// header that would be available in all Standard modes, and we don't want to
// break that use case.
# if __has_include_next(<stdatomic.h>)
#   include_next <stdatomic.h>
# endif

#endif // _LIBCUDACXX_STD_VER > 20

#endif // _LIBCUDACXX_STDATOMIC_H
