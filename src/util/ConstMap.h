#ifndef KOOLANG_CONSTMAP_H
#define KOOLANG_CONSTMAP_H

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
#include <utility>

template <typename Key, typename Value, std::size_t Size> struct ConstMap {
    using ConstMapPair  = std::pair<Key, Value>;
    using ConstMapPairs = std::array<ConstMapPair, Size>;

    ConstMapPairs Data;

    [[nodiscard]] constexpr Value get(const Key& key, const Value& defaultVal) const {
        for (std::size_t i = 0; i < Size; i++) {
            if (Data.at(i).first == key) {
                return Data.at(i).second;
            }
        }
        return defaultVal;
    }
};

#endif // KOOLANG_CONSTMAP_H
