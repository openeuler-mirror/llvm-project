//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <__config>
#include <fstream>
#include <ios>
#include <istream>
#include <ostream>
#include <sstream>
#include <streambuf>

_LIBCUDACXX_BEGIN_NAMESPACE_STD

// Original explicit instantiations provided in the library
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ios<char>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_streambuf<char>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_istream<char>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ostream<char>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_iostream<char>;

#ifndef _LIBCUDACXX_HAS_NO_WIDE_CHARACTERS
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ios<wchar_t>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_streambuf<wchar_t>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_istream<wchar_t>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ostream<wchar_t>;
#endif

// Additional instantiations added later. Whether programs rely on these being
// available is protected by _LIBCUDACXX_ABI_ENABLE_ADDITIONAL_IOSTREAM_EXPLICIT_INSTANTIATIONS_1.
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_stringbuf<char>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_stringstream<char>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ostringstream<char>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_istringstream<char>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ifstream<char>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_ofstream<char>;
template class _LIBCUDACXX_CLASS_TEMPLATE_INSTANTIATION_VIS basic_filebuf<char>;

// Add more here if needed...

_LIBCUDACXX_END_NAMESPACE_STD
