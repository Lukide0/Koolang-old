#ifndef KOOLANG_AIR_VALUE_H
#define KOOLANG_AIR_VALUE_H

#include "util/Index.h"
#include <cstdint>
namespace air::value {

enum class ResultState {
    OK,
    UNDERFLOW,
    OVERFLOW,
    SHIFT_NEGATIVE,
};
template <typename T> struct Result {

    T Val;
    ResultState State;

    [[nodiscard]] bool HasErr() const { return State != ResultState::OK; }
};

Result<std::uint64_t> addSigned(std::uint64_t a, std::uint64_t b);
Result<std::uint64_t> addUnsigned(std::uint64_t a, std::uint64_t b);

Result<std::uint64_t> subSigned(std::uint64_t a, std::uint64_t b);
Result<std::uint64_t> subUnsigned(std::uint64_t a, std::uint64_t b);

Result<std::uint64_t> mulSigned(std::uint64_t a, std::uint64_t b);
Result<std::uint64_t> mulUnsigned(std::uint64_t a, std::uint64_t b);

Result<std::uint64_t> divSigned(std::uint64_t a, std::uint64_t b);
Result<std::uint64_t> divUnsigned(std::uint64_t a, std::uint64_t b);

Result<std::uint64_t> modSigned(std::uint64_t a, std::uint64_t b);
Result<std::uint64_t> modUnsigned(std::uint64_t a, std::uint64_t b);

bool canFitInt(Index type, std::uint64_t val);
}

#endif
