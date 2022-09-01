// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// The BSDs have lots of *_l functions.  This file provides reimplementations
// of those functions for non-BSD platforms.
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___BSD_LOCALE_FALLBACKS_H
#define _LIBCUDACXX___BSD_LOCALE_FALLBACKS_H

#include <memory>
#include <stdarg.h>
#include <stdlib.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

inline _LIBCUDACXX_INLINE_VISIBILITY
decltype(MB_CUR_MAX) __LIBCUDACXX_mb_cur_max_l(locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return MB_CUR_MAX;
}

#ifndef _LIBCUDACXX_HAS_NO_WIDE_CHARACTERS
inline _LIBCUDACXX_INLINE_VISIBILITY
wint_t __LIBCUDACXX_btowc_l(int __c, locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return btowc(__c);
}

inline _LIBCUDACXX_INLINE_VISIBILITY
int __LIBCUDACXX_wctob_l(wint_t __c, locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return wctob(__c);
}

inline _LIBCUDACXX_INLINE_VISIBILITY
size_t __LIBCUDACXX_wcsnrtombs_l(char *__dest, const wchar_t **__src, size_t __nwc,
                         size_t __len, mbstate_t *__ps, locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return wcsnrtombs(__dest, __src, __nwc, __len, __ps);
}

inline _LIBCUDACXX_INLINE_VISIBILITY
size_t __LIBCUDACXX_wcrtomb_l(char *__s, wchar_t __wc, mbstate_t *__ps, locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return wcrtomb(__s, __wc, __ps);
}

inline _LIBCUDACXX_INLINE_VISIBILITY
size_t __LIBCUDACXX_mbsnrtowcs_l(wchar_t * __dest, const char **__src, size_t __nms,
                      size_t __len, mbstate_t *__ps, locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return mbsnrtowcs(__dest, __src, __nms, __len, __ps);
}

inline _LIBCUDACXX_INLINE_VISIBILITY
size_t __LIBCUDACXX_mbrtowc_l(wchar_t *__pwc, const char *__s, size_t __n,
                   mbstate_t *__ps, locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return mbrtowc(__pwc, __s, __n, __ps);
}

inline _LIBCUDACXX_INLINE_VISIBILITY
int __LIBCUDACXX_mbtowc_l(wchar_t *__pwc, const char *__pmb, size_t __max, locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return mbtowc(__pwc, __pmb, __max);
}

inline _LIBCUDACXX_INLINE_VISIBILITY
size_t __LIBCUDACXX_mbrlen_l(const char *__s, size_t __n, mbstate_t *__ps, locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return mbrlen(__s, __n, __ps);
}
#endif // _LIBCUDACXX_HAS_NO_WIDE_CHARACTERS

inline _LIBCUDACXX_INLINE_VISIBILITY
lconv *__LIBCUDACXX_localeconv_l(locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return localeconv();
}

#ifndef _LIBCUDACXX_HAS_NO_WIDE_CHARACTERS
inline _LIBCUDACXX_INLINE_VISIBILITY
size_t __LIBCUDACXX_mbsrtowcs_l(wchar_t *__dest, const char **__src, size_t __len,
                     mbstate_t *__ps, locale_t __l)
{
    __LIBCUDACXX_locale_guard __current(__l);
    return mbsrtowcs(__dest, __src, __len, __ps);
}
#endif

inline _LIBCUDACXX_ATTRIBUTE_FORMAT(__printf__, 4, 5)
int __LIBCUDACXX_snprintf_l(char *__s, size_t __n, locale_t __l, const char *__format, ...) {
    va_list __va;
    va_start(__va, __format);
    __LIBCUDACXX_locale_guard __current(__l);
    int __res = vsnprintf(__s, __n, __format, __va);
    va_end(__va);
    return __res;
}

inline _LIBCUDACXX_ATTRIBUTE_FORMAT(__printf__, 3, 4)
int __LIBCUDACXX_asprintf_l(char **__s, locale_t __l, const char *__format, ...) {
    va_list __va;
    va_start(__va, __format);
    __LIBCUDACXX_locale_guard __current(__l);
    int __res = vasprintf(__s, __format, __va);
    va_end(__va);
    return __res;
}

inline _LIBCUDACXX_ATTRIBUTE_FORMAT(__scanf__, 3, 4)
int __LIBCUDACXX_sscanf_l(const char *__s, locale_t __l, const char *__format, ...) {
    va_list __va;
    va_start(__va, __format);
    __LIBCUDACXX_locale_guard __current(__l);
    int __res = vsscanf(__s, __format, __va);
    va_end(__va);
    return __res;
}

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___BSD_LOCALE_FALLBACKS_H
