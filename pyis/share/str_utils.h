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

void FindAndReplaceAll(std::string& data, const std::string& str_to_search, const std::string& str_to_replace);

void unescape_string(std::string& str);

class ustring : public std::u32string {
  public:
    ustring();
    explicit ustring(char* str);
    explicit ustring(const char* str);
    explicit ustring(std::string& str);
    explicit ustring(const std::string& str);
    explicit ustring(char32_t* str);
    explicit ustring(const char32_t* str);
    explicit ustring(std::u32string& str);
    explicit ustring(std::u32string&& str);
    explicit ustring(const std::u32string& str);
    explicit ustring(const std::u32string&& str);

    explicit operator std::string();
    explicit operator std::string() const;

  private:
    using utf8_converter = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>;
};


}  // namespace pyis

namespace std {
template <>
struct hash<pyis::ustring> {
    size_t operator()(const pyis::ustring& __str) const noexcept {
        hash<u32string> standard_hash;
        return standard_hash(static_cast<u32string>(__str));
    }
};
}  // namespace std

bool isUnicodeCategoryL(char32_t ch);

bool isUnicodeCategoryN(char32_t ch);

bool isUnicodeCategoryZ(char32_t ch);

bool NotLNZ(char32_t ch);