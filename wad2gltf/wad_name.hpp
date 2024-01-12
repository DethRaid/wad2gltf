#pragma once

#define __STDC_WANT_LIB_EXT1__ 1
#include <cstring>
#include <cctype>
#include <format>
#include <string_view>

inline size_t strlen_s_wrapper(const char* str, size_t max_length) {
#ifdef __STDC_LIB_EXT1__
    return strlen_s(str, max_length);
#else
    for (auto size = size_t{0}; size < max_length; size++) {
        if (str[size] == '\0') {
            return size;
        }
    }

    return max_length;
#endif
}

namespace wad {
    struct Name {
        char val[8] = {0, 0, 0, 0, 0, 0, 0, 0};

        bool operator==(std::string_view str) const;

        bool operator==(const Name& other) const;

        std::string to_string() const;

        bool is_valid() const;

        bool is_none() const;

        bool starts_with(std::string_view prefix) const;
    };

    inline bool Name::operator==(const std::string_view str) const {
        return memcmp(val, str.data(), std::min(str.size(), static_cast<size_t>(8))) == 0;
    }

    inline bool Name::operator==(const Name& other) const {
        for (auto i = 0; i < 8; i++) {
            if (toupper(static_cast<unsigned char>(val[i])) != toupper(static_cast<unsigned char>(other.val[i]))) {
                return false;
            }

            if (val[i] == other.val[i] && val[i] == '\0') {
                // We've iterated to the end of the string, and we've reached the null terminator. The names are equal
                return true;
            }
        }

        // We reached the end of the string without finding different characters. These are both eight-character names
        return true;
    }

    inline std::string Name::to_string() const {
        const auto length = strlen_s_wrapper(val, 8);
        return std::string{val, length};
    }

    inline bool Name::is_valid() const {
        return val[0] != '\0';
    }

    inline bool Name::is_none() const {
        // The Unofficial Doom Specs state that '-' means "no texture" or transparent (not rendered)
        return val[0] == '-' && val[1] == '\0';
    }

    inline bool Name::starts_with(const std::string_view prefix) const {
        if(prefix.size() > 8) {
            return false;
        }

        return memcmp(val, prefix.data(), prefix.size()) == 0;
    }
}

template <>
struct std::formatter<wad::Name> : std::formatter<std::string> {
    auto format(const wad::Name& name, format_context& ctx) const {
        const auto length = strlen_s_wrapper(name.val, 8);
        const auto view = std::string_view{ name.val, length };
        return formatter<string>::format(std::format("{}", view), ctx);
    }
};

template <>
struct std::hash<wad::Name> {
    std::size_t operator()(const wad::Name& name) const noexcept {
        return std::hash<std::string>{}(name.to_string());
    }
};
