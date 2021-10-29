// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "str_utils.h"

namespace pyis {

std::vector<std::string> split_str(const std::string& s, const char* delim, bool remove_empty) {
    std::vector<std::string> res;
    if (s.length() == 0) {
        return res;
    }

    auto start = 0U;
    auto end = s.find_first_of(delim);
    while (end != std::string::npos) {
        std::string token = s.substr(start, end - start);
        if (!remove_empty || !token.empty()) {
            res.emplace_back(token);
        }
        start = end + 1;
        end = s.find_first_of(delim, start);
    }

    if (start < s.length()) {
        std::string token = s.substr(start);
        res.emplace_back(token);
    }

    return res;
}

std::string to_lower(const std::string& s) {
    std::string lowercase_str;
    std::transform(s.begin(), s.end(), lowercase_str.begin(), [](unsigned char c) { return std::tolower(c); });
    return lowercase_str;
}

std::string join_str(const std::vector<std::string>& tokens, const std::string& delim) {
    std::string res;
    for (auto p = tokens.begin(); p != tokens.end(); ++p) {
        res += *p;
        if (p != tokens.end() - 1) {
            res += delim;
        }
    }

    return res;
}

std::string& ltrim_str(std::string& str, const std::string& chars) {
    str.erase(0, str.find_first_not_of(chars));
    return str;
}

std::string& rtrim_str(std::string& str, const std::string& chars) {
    str.erase(str.find_last_not_of(chars) + 1);
    return str;
}

std::string& trim_str(std::string& str, const std::string& chars) { return ltrim_str(rtrim_str(str, chars), chars); }

bool str_ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) {
        return false;
    }
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}

#ifndef USE_FMTLIB
std::string fmt_str(const char* format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    buffer[sizeof(buffer) - 1] = '\0';
    va_end(args);
    return std::string(buffer);
}
#endif

std::wstring str_to_wstr(const std::string& str) {
    if (str.empty()) {
        return std::wstring();
    }
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::wstring result = converter.from_bytes(str.data(), str.data() + str.size());
    return result;
}

std::string wstr_to_str(const std::wstring& wstr) {
    if (wstr.empty()) {
        return std::string();
    }
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string result = converter.to_bytes(wstr.data(), wstr.data() + wstr.size());
    return result;
}

bool isCJK(char32_t c) {
    return (c >= 0x4E00 && c <= 0x9FFF) || (c >= 0x3400 && c <= 0x4DBF) || (c >= 0x20000 && c <= 0x2A6DF) ||
           (c >= 0x2A700 && c <= 0x2B73F) || (c >= 0x2B740 && c <= 0x2B81F) || (c >= 0x2B820 && c <= 0x2CEAF) ||
           (c >= 0xF900 && c <= 0xFAFF) || (c >= 0x2F800 && c <= 0x2FA1F);
}

bool IsAccent(char32_t c) {
    // only support part of accent
    // [TODO] support more accent
    return c >= 0x300 && c <= 0x36F;
}

char32_t StripAccent(char32_t c) {
    //   "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"
    const char* tr = "AAAAAAÆCEEEEIIIIÐNOOOOO×ØUUUUYÞßaaaaaaæceeeeiiiiðnooooo÷øuuuuyþy";
    if (c < 192 || c > 255) {
        return c;
    }

    return tr[c - 192];
}

void findAndReplaceAll(std::string& data, const std::string& str_to_search, const std::string& str_to_replace) {
    size_t pos = data.find(str_to_search);
    while (pos != std::string::npos) {
        data.replace(pos, str_to_search.length(), str_to_replace);
        pos = data.find(str_to_search, pos + str_to_replace.length());
    }
}

}  // namespace pyis
