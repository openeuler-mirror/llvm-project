// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FUNCTIONAL_DEFAULT_SEARCHER_H
#define _LIBCUDACXX___FUNCTIONAL_DEFAULT_SEARCHER_H

#include <__algorithm/search.h>
#include <__config>
#include <__functional/identity.h>
#include <__functional/operations.h>
#include <__iterator/iterator_traits.h>
#include <__utility/pair.h>

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 14

// default searcher
template<class _ForwardIterator, class _BinaryPredicate = equal_to<>>
class _LIBCUDACXX_TEMPLATE_VIS default_searcher {
public:
    _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
    default_searcher(_ForwardIterator __f, _ForwardIterator __l,
                       _BinaryPredicate __p = _BinaryPredicate())
        : __first_(__f), __last_(__l), __pred_(__p) {}

    template <typename _ForwardIterator2>
    _LIBCUDACXX_INLINE_VISIBILITY _LIBCUDACXX_CONSTEXPR_AFTER_CXX17
    pair<_ForwardIterator2, _ForwardIterator2>
    operator () (_ForwardIterator2 __f, _ForwardIterator2 __l) const
    {
        auto __proj = __identity();
        return std::__search_impl(__f, __l, __first_, __last_, __pred_, __proj, __proj);
    }

private:
    _ForwardIterator __first_;
    _ForwardIterator __last_;
    _BinaryPredicate __pred_;
};

#endif // _LIBCUDACXX_STD_VER > 14

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___FUNCTIONAL_DEFAULT_SEARCHER_H
