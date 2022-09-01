//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <regex>

// class regex_error
//     : public runtime_error
// {
// public:
//     explicit regex_error(regex_constants::error_type ecode);
//     regex_constants::error_type code() const;
// };

#include <regex>
#include <cassert>
#include "test_macros.h"

int main(int, char**)
{
    {
        std::regex_error e(std::regex_constants::error_collate);
        assert(e.code() == std::regex_constants::error_collate);
        LIBCUDACXX_ASSERT(e.what() == std::string("The expression contained an invalid collating element name."));
    }
    {
        std::regex_error e(std::regex_constants::error_ctype);
        assert(e.code() == std::regex_constants::error_ctype);
        LIBCUDACXX_ASSERT(e.what() == std::string("The expression contained an invalid character class name."));
    }
    {
        std::regex_error e(std::regex_constants::error_escape);
        assert(e.code() == std::regex_constants::error_escape);
        LIBCUDACXX_ASSERT(e.what() == std::string("The expression contained an invalid escaped character, or a "
               "trailing escape."));
    }
    {
        std::regex_error e(std::regex_constants::error_backref);
        assert(e.code() == std::regex_constants::error_backref);
        LIBCUDACXX_ASSERT(e.what() == std::string("The expression contained an invalid back reference."));
    }
    {
        std::regex_error e(std::regex_constants::error_brack);
        assert(e.code() == std::regex_constants::error_brack);
        LIBCUDACXX_ASSERT(e.what() == std::string("The expression contained mismatched [ and ]."));
    }
    {
        std::regex_error e(std::regex_constants::error_paren);
        assert(e.code() == std::regex_constants::error_paren);
        LIBCUDACXX_ASSERT(e.what() == std::string("The expression contained mismatched ( and )."));
    }
    {
        std::regex_error e(std::regex_constants::error_brace);
        assert(e.code() == std::regex_constants::error_brace);
        LIBCUDACXX_ASSERT(e.what() == std::string("The expression contained mismatched { and }."));
    }
    {
        std::regex_error e(std::regex_constants::error_badbrace);
        assert(e.code() == std::regex_constants::error_badbrace);
        LIBCUDACXX_ASSERT(e.what() == std::string("The expression contained an invalid range in a {} expression."));
    }
    {
        std::regex_error e(std::regex_constants::error_range);
        assert(e.code() == std::regex_constants::error_range);
        LIBCUDACXX_ASSERT(e.what() == std::string("The expression contained an invalid character range, "
               "such as [b-a] in most encodings."));
    }
    {
        std::regex_error e(std::regex_constants::error_space);
        assert(e.code() == std::regex_constants::error_space);
        LIBCUDACXX_ASSERT(e.what() == std::string("There was insufficient memory to convert the expression into "
               "a finite state machine."));
    }
    {
        std::regex_error e(std::regex_constants::error_badrepeat);
        assert(e.code() == std::regex_constants::error_badrepeat);
        LIBCUDACXX_ASSERT(e.what() == std::string("One of *?+{ was not preceded by a valid regular expression."));
    }
    {
        std::regex_error e(std::regex_constants::error_complexity);
        assert(e.code() == std::regex_constants::error_complexity);
        LIBCUDACXX_ASSERT(e.what() == std::string("The complexity of an attempted match against a regular "
               "expression exceeded a pre-set level."));
    }
    {
        std::regex_error e(std::regex_constants::error_stack);
        assert(e.code() == std::regex_constants::error_stack);
        LIBCUDACXX_ASSERT(e.what() == std::string("There was insufficient memory to determine whether the regular "
               "expression could match the specified character sequence."));
    }

  return 0;
}
