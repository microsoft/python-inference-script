// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.

#include "ustring.h"

namespace pyis {

Ustring::Ustring() = default;

Ustring::Ustring(char* str) {
    utf8_converter str_cvt;
    assign(str_cvt.from_bytes(str));
}

Ustring::Ustring(const char* str) {
    utf8_converter str_cvt;
    assign(str_cvt.from_bytes(str));
}

Ustring::Ustring(std::string& str) {
    utf8_converter str_cvt;
    assign(str_cvt.from_bytes(str));
}

Ustring::Ustring(const std::string& str) {
    utf8_converter str_cvt;
    assign(str_cvt.from_bytes(str));
}

Ustring::Ustring(char32_t* str) : std::u32string(str) {}

Ustring::Ustring(const char32_t* str) : std::u32string(str) {}

Ustring::Ustring(std::u32string& str) : std::u32string(str) {}

Ustring::Ustring(std::u32string&& str) : std::u32string(str) {}

Ustring::Ustring(const std::u32string& str) : std::u32string(str) {}

Ustring::Ustring(const std::u32string&& str) : std::u32string(str) {}

Ustring::operator std::string() {
    utf8_converter str_cvt;
    return str_cvt.to_bytes(*this);
}

Ustring::operator std::string() const {
    utf8_converter str_cvt;
    return str_cvt.to_bytes(*this);
}
}  // namespace pyis