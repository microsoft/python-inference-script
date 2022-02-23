// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license.
#include <codecvt>
#include <locale>
#include <string>

namespace pyis {
class Ustring : public std::u32string {
  public:
    Ustring();
    explicit Ustring(char* str);
    explicit Ustring(const char* str);
    explicit Ustring(std::string& str);
    explicit Ustring(const std::string& str);
    explicit Ustring(char32_t* str);
    explicit Ustring(const char32_t* str);
    explicit Ustring(std::u32string& str);
    explicit Ustring(std::u32string&& str);
    explicit Ustring(const std::u32string& str);
    explicit Ustring(const std::u32string&& str);

    explicit operator std::string();
    explicit operator std::string() const;

  private:
    using utf8_converter = std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t>;
};
}  // namespace pyis

namespace std {
template <>
struct hash<pyis::Ustring> {
    size_t operator()(const pyis::Ustring& str) const noexcept {
        hash<u32string> standard_hash;
        return standard_hash(static_cast<u32string>(str));
    }
};
}  // namespace std
