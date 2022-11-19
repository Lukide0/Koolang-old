#ifndef KOOLANG_UTIL_STRING_HASH_H
#define KOOLANG_UTIL_STRING_HASH_H

#include <functional>
#include <string_view>

struct string_hash {
    using hash_type      = std::hash<std::string_view>;
    using is_transparent = int;

    [[nodiscard]] size_t operator()(const char* text) const { return hash_type {}(text); }
    [[nodiscard]] size_t operator()(std::string_view text) const { return hash_type {}(text); }
};

#endif
