//===- WeakConsistencyAllowlist.cpp - Weak Consistency Pass ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
 
#include "WeakConsistencyAllowlist.h"
#include "WeakConsistencyConfig.h"
#include "llvm/Demangle/Demangle.h"
#include <climits>
#include <cstdlib>
#include <fstream>
#ifdef WIN32
#include <fileapi.h>
#endif
#include <string>
 
namespace {
struct Path {
    explicit Path(const std::string &base) : ori(base) {}
 
    std::string Realpath() const;
 
private:
    std::string ori;
};
 
std::string Path::Realpath() const {
    char tempBuf[PATH_MAX] = {0x00};
#ifdef WIN32
    GetFullPathName(ori.c_str(), PATH_MAX, tempBuf, NULL);
#else
    if (realpath(ori.c_str(), tempBuf) == nullptr)
        return "";
#endif
    return tempBuf;
}
 
std::string &trim(std::string &s) {
    const std::string WHITESPACE = " \n\r\t\f\v";
    if (s.empty()) 
        return s;
    s.erase(0, s.find_first_not_of(WHITESPACE));
    s.erase(s.find_last_not_of(WHITESPACE) + 1);
    return s;
}
 
std::string getFuncnameFromFile(const std::string &func) {
    std::string ret(func);
    auto iter = ret.find_last_of('(');
    if (iter != std::string::npos) 
        ret.erase(iter);
    trim(ret);
    iter = ret.find_last_of(" \t");
    if (iter != std::string::npos) 
        ret.erase(0, iter + 1);
    iter = ret.find_last_of(':');
    if (iter != std::string::npos)
        ret.erase(0, iter + 1);
    return ret;
}
}
 
std::string WeakConsistencyAllowlist::getFunctionName(const std::string &mangledName) {
    llvm::ItaniumPartialDemangler IPD;
    if (IPD.partialDemangle(mangledName.c_str())) 
        return mangledName;
    size_t n = ipdSize;
    auto res = IPD.getFunctionBaseName(ipdBuf, &n);
    if (res == nullptr) 
        return mangledName;
    if (res != ipdBuf) {
        ipdBuf = res;
        ipdSize = n;
    }
    return ipdBuf;
}
 
// Permitted list format
// The file list starts with "files: "and occupies one line exclusively.
// The function list starts with "functions: "and occupies one line exclusively.
// Spaces are allowed at the beginning and end.
bool WeakConsistencyAllowlist::parseTag(std::string &tag) {
    tag.pop_back();
    trim(tag);
    if (tag == FUNCTION_TAG) 
        parseState = PARSE_FUNC;
    else if (tag == FILE_TAG) 
        parseState = PARSE_FILE;
    else 
        return false;
    return true;
}
 
bool WeakConsistencyAllowlist::addFile(std::string &line)
{
    std::string realPath = Path(line).Realpath();
    if (realPath.empty()) 
        return true;
    files.insert(realPath);
    return true;
}
 
bool WeakConsistencyAllowlist::addFunc(std::string &line)
{
    auto funcname = getFuncnameFromFile(line);
    if (funcname.empty()) 
        return true;
    funcs.insert(funcname);
    return true;
}
 
bool WeakConsistencyAllowlist::parseLine(std::string &line) {
    trim(line);
    if (line.empty()) 
        return true;
    if (line.back() == END_TAG) 
        return parseTag(line);
    switch (parseState) {
        case PARSE_FILE:
            return addFile(line);
        case PARSE_FUNC:
            return addFunc(line);
        case PARSE_NONE:
            return false;
    }
    return false;
}
 
bool WeakConsistencyAllowlist::Initialize(const std::string &filename)
{
    std::string buf;
    std::string realPath = Path(filename).Realpath();
    if (realPath.empty()) 
        return true;
    std::ifstream in(realPath);
    if (in.fail()) {
        llvm::WithColor::error(llvm::errs(), "WeakConsistencyPass")
             << "'WeakConsistency allowlist' open failed: permission error.\n";
        return false;
    }
    hasAllowlist = true;
    bool ret = true;
    while (std::getline(in, buf)) {
        if (!parseLine(buf)) {
            llvm::WithColor::error(llvm::errs(), "WeakConsistencyPass")
                << "'WeakConsistency allowlist' read failed: format error.\n";
            ret = false;
            break;
        }
    }
    in.close();
    return ret;
}
 
bool WeakConsistencyAllowlist::Check(const std::string &filename, const std::string &funcname)
{
    if (!hasAllowlist) 
        return true;
    std::string realPath = Path(filename).Realpath();
    if (files.find(realPath) != files.end()) 
        return true;
    std::string realFunc = getFunctionName(funcname);
    return funcs.find(realFunc) != funcs.end();
}


