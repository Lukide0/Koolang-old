#ifndef KOOLANG_UTIL_ARRAY_UTIL_H
#define KOOLANG_UTIL_ARRAY_UTIL_H

#include "util/Index.h"
#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

template <typename T> inline void freeVecMemory(std::vector<T>& vec) {
    // shrink_to_fit is non-binding request !!
    std::vector<T>().swap(vec);
}

template <typename T, std::size_t... N> constexpr auto concatArrs(const std::array<T, N>&... arr) {
    constexpr std::size_t SIZE = (N + ...);
    return std::apply([](auto... n) { return std::array<T, SIZE> { n... }; }, std::tuple_cat(arr...));
}

template <typename T, std::size_t N> constexpr bool arrContains(const std::array<T, N>& arr, const T& val) {
    return std::find(arr.begin(), arr.end(), val) != arr.end();
}

template <typename Type> inline void serializeToVec(std::vector<Index>& vec, Type value) {
    // check if the value can be serialized
    ASSERT_INDEX_SERIALIZABLE(Type);

    constexpr std::size_t fieldsCount = sizeof(Type) / sizeof(Index);
    const Index* ptr                  = reinterpret_cast<Index*>(&value);

    vec.insert(vec.end(), ptr, ptr + fieldsCount);
}

template <typename Type> inline Type deserializeFromVec(const std::vector<Index>& vec, Index begin) {
    // check if the value can be deserialized
    ASSERT_INDEX_SERIALIZABLE(Type);

    constexpr std::size_t fieldsCount = sizeof(Type) / sizeof(Index);
    // check vector
    assert(fieldsCount + begin <= vec.size());

    const auto* tmpVal = reinterpret_cast<const Type*>(vec.data() + begin);

    return *tmpVal;
}

#endif
