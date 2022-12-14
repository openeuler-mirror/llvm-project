//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// <random>

// template<class RealType = double>
// class extreme_value_distribution

// result_type min() const;

#include <random>

#include <cassert>
#include <cmath>

#include "test_macros.h"

int main(int, char**)
{
    {
        typedef std::extreme_value_distribution<> D;
        D d(.5, .5);
        assert(d.min() == -INFINITY);
    }

  return 0;
}
