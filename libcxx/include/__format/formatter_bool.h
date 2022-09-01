// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCUDACXX___FORMAT_FORMATTER_BOOL_H
#define _LIBCUDACXX___FORMAT_FORMATTER_BOOL_H

#include <__algorithm/copy.h>
#include <__availability>
#include <__config>
#include <__debug>
#include <__format/format_error.h>
#include <__format/format_fwd.h>
#include <__format/format_parse_context.h>
#include <__format/formatter.h>
#include <__format/formatter_integral.h>
#include <__format/parser_std_format_spec.h>
#include <__utility/unreachable.h>
#include <string_view>

#ifndef _LIBCUDACXX_HAS_NO_LOCALIZATION
#  include <locale>
#endif

#if !defined(_LIBCUDACXX_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCUDACXX_BEGIN_NAMESPACE_STD

#if _LIBCUDACXX_STD_VER > 17

template <__formatter::__char_type _CharT>
struct _LIBCUDACXX_TEMPLATE_VIS _LIBCUDACXX_AVAILABILITY_FORMAT formatter<bool, _CharT> {
public:
  _LIBCUDACXX_HIDE_FROM_ABI constexpr auto
  parse(basic_format_parse_context<_CharT>& __parse_ctx) -> decltype(__parse_ctx.begin()) {
    auto __result = __parser_.__parse(__parse_ctx, __format_spec::__fields_integral);
    __format_spec::__process_parsed_bool(__parser_);
    return __result;
  }

  _LIBCUDACXX_HIDE_FROM_ABI auto format(bool __value, auto& __ctx) const -> decltype(__ctx.out()) {
    switch (__parser_.__type_) {
    case __format_spec::__type::__default:
    case __format_spec::__type::__string:
      return __formatter::__format_bool(__value, __ctx, __parser_.__get_parsed_std_specifications(__ctx));

    case __format_spec::__type::__binary_lower_case:
    case __format_spec::__type::__binary_upper_case:
    case __format_spec::__type::__octal:
    case __format_spec::__type::__decimal:
    case __format_spec::__type::__hexadecimal_lower_case:
    case __format_spec::__type::__hexadecimal_upper_case:
      // Promotes bool to an integral type. This reduces the number of
      // instantiations of __format_integer reducing code size.
      return __formatter::__format_integer(
          static_cast<unsigned>(__value), __ctx, __parser_.__get_parsed_std_specifications(__ctx));

    default:
      _LIBCUDACXX_ASSERT(false, "The parse function should have validated the type");
      __LIBCUDACXX_unreachable();
    }
  }

  __format_spec::__parser<_CharT> __parser_;
};

#endif //_LIBCUDACXX_STD_VER > 17

_LIBCUDACXX_END_NAMESPACE_STD

#endif // _LIBCUDACXX___FORMAT_FORMATTER_BOOL_H
