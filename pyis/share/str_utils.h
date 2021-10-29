// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#pragma once

#ifdef USE_FMTLIB
#include <fmt/printf.h>
#else
#include <cstdarg>
#include <cstdlib>
#endif

#include <algorithm>
#include <codecvt>
#include <cstdint>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace pyis {

std::vector<std::string> split_str(const std::string& s, const char* delim = " \r\n\t", bool remove_empty = true);

std::string join_str(const std::vector<std::string>& tokens, const std::string& delim);

std::string& ltrim_str(std::string& str, const std::string& chars = "\t\n\v\f\r ");

std::string& rtrim_str(std::string& str, const std::string& chars = "\t\n\v\f\r ");

std::string& trim_str(std::string& str, const std::string& chars = "\t\n\v\f\r ");

bool str_ends_with(std::string const& str, std::string const& suffix);

std::string to_lower(const std::string& s);

#ifdef USE_FMTLIB
template <typename... Args>
std::string fmt_str(const char* fmt, Args&&... args) {
    return fmt::sprintf(fmt, std::forward<Args>(args)...);
}
#else
std::string fmt_str(const char* format, ...);
#endif

std::wstring str_to_wstr(const std::string& str);

std::string wstr_to_str(const std::wstring& wstr);

bool isCJK(char32_t c);

bool IsAccent(char32_t c);

char32_t StripAccent(char32_t c);

void findAndReplaceAll(std::string& data, const std::string& toSearch, const std::string& replaceStr);

}  // namespace pyis
