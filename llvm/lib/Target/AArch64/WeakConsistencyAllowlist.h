//===- WeakConsistencyAllowlist.h - Weak Consistency Pass ----------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
 
#ifndef LLVM_LIB_TARGET_AARCH64_WEAKCONSISTENCYALLOWLIST_H
#define LLVM_LIB_TARGET_AARCH64_WEAKCONSISTENCYALLOWLIST_H
 
#include <string>
#include <unordered_set>
#include <cassert>
 
class WeakConsistencyAllowlist {
public:
    WeakConsistencyAllowlist() {
        ipdBuf = static_cast<char *>(std::malloc(ipdSize));
        assert(ipdBuf);
        ipdBuf[ipdSize-1] = '\0';
    }
    ~WeakConsistencyAllowlist() {
        if(ipdBuf != nullptr) {
            free(ipdBuf);
        }
    }
 
    bool Initialize(const std::string &filename);
    bool Check(const std::string &filename, const std::string &funcname);
 
private:
    bool parseLine(std::string &line);
    bool parseTag(std::string &tag);
    bool addFile(std::string &line);
    bool addFunc(std::string &line);
    std::string getFunctionName(const std::string &mangledName);
 
private:
    enum ParseState : uint8_t {
        PARSE_NONE,
        PARSE_FILE,
        PARSE_FUNC,
    };
 
    bool hasAllowlist = false;
    std::unordered_set<std::string> files = {};
    std::unordered_set<std::string> funcs = {};
    const std::string FILE_TAG = "files";
    const std::string FUNCTION_TAG = "functions";
    const char END_TAG = ':';
    ParseState parseState = PARSE_NONE;
    char *ipdBuf = nullptr;
    size_t ipdSize = 2048;
};
 
#endif

