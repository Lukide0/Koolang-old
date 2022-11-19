#ifndef KOOLANG_UTIL_HASH_COMBINE_H
#define KOOLANG_UTIL_HASH_COMBINE_H

#include <cstddef>
#include <cstdint>
#include <functional>

// https://github.com/boostorg/container_hash/blob/develop/include/boost/container_hash/hash.hpp

inline void hash_combine([[maybe_unused]] std::size_t& seed) { }

// 64bit hash combine
template <typename T, typename... Rest> inline void hash_combine(std::size_t& seed, const T& v, Rest... rest) {
    constexpr std::uint64_t m = (std::uint64_t(0xe9846af) << 32) + 0x9b1a615d;

    std::hash<T> hasher;
    seed += 0x9e3779b9 + hasher(v);

    seed ^= seed >> 32;
    seed *= m;
    seed ^= seed >> 32;
    seed *= m;
    seed ^= seed >> 28;

    hash_combine(seed, rest...);
}

#endif
