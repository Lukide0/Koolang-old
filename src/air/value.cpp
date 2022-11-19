#include "value.h"
#include "Pool.h"
#include "type.h"
#include <limits>

namespace air::value {

// https://vladris.com/blog/2018/10/13/arithmetic-overflow-and-underflow.html

template <typename T> constexpr inline bool isAddOverflow(const T& a, const T& b) {
    return (b >= 0) && (a > std::numeric_limits<T>::max() - b);
}

template <typename T> constexpr inline bool isAddUnderflow(const T& a, const T& b) {
    return (b < 0) && (a < std::numeric_limits<T>::min() - b);
}

template <typename T> constexpr inline bool isSubOverflow(const T& a, const T& b) {
    return (b < 0) && (a > std::numeric_limits<T>::max() + b);
}

template <typename T> constexpr inline bool isSubUnderflow(const T& a, const T& b) {
    return (b >= 0) && (a < std::numeric_limits<T>::min() + b);
}

template <typename T> constexpr bool isMulOverflow(const T& a, const T& b) {
    if (b == 0) {
        return false; // Avoid division by 0
    }
    return ((b > 0) && (a > 0) && (a > std::numeric_limits<T>::max() / b))
        || ((b < 0) && (a < 0) && (a < std::numeric_limits<T>::max() / b));
}

template <typename T> constexpr bool isMulUnderflow(const T& a, const T& b) {
    if (b == 0) {
        return false; // Avoid division by 0
    }
    return ((b > 0) && (a < 0) && (a < std::numeric_limits<T>::min() / b))
        || ((b < 0) && (a > 0) && (a > std::numeric_limits<T>::min() / b));
}

Result<std::uint64_t> addSigned(std::uint64_t a, std::uint64_t b) {
    auto signedA = static_cast<std::int64_t>(a);
    auto signedB = static_cast<std::int64_t>(b);

    if (isAddOverflow(signedA, signedB)) {
        return { 0, ResultState::OVERFLOW };
    } else if (isAddUnderflow(signedA, signedB)) {
        return { 0, ResultState::UNDERFLOW };
    } else {
        return { static_cast<std::uint64_t>(signedA + signedB), ResultState::OK };
    }
}

Result<std::uint64_t> addUnsigned(std::uint64_t a, std::uint64_t b) {
    if (isAddOverflow(a, b)) {
        return { 0, ResultState::OVERFLOW };
    } else {
        return { a + b, ResultState::OK };
    }
}

Result<std::uint64_t> subSigned(std::uint64_t a, std::uint64_t b) {
    auto signedA = static_cast<std::int64_t>(a);
    auto signedB = static_cast<std::int64_t>(b);

    if (isSubOverflow(signedA, signedB)) {
        return { 0, ResultState::OVERFLOW };
    } else if (isSubUnderflow(signedA, signedB)) {
        return { 0, ResultState::UNDERFLOW };
    } else {
        return { static_cast<std::uint64_t>(signedA - signedB), ResultState::OK };
    }
}

Result<std::uint64_t> subUnsigned(std::uint64_t a, std::uint64_t b) {
    if (isSubOverflow(a, b)) {
        return { 0, ResultState::OVERFLOW };
    } else if (isSubUnderflow(a, b)) {
        return { 0, ResultState::UNDERFLOW };
    } else {
        return { a - b, ResultState::OK };
    }
}

Result<std::uint64_t> mulSigned(std::uint64_t a, std::uint64_t b) {
    auto signedA = static_cast<std::int64_t>(a);
    auto signedB = static_cast<std::int64_t>(b);

    if (isMulOverflow(signedA, signedB)) {
        return { 0, ResultState::OVERFLOW };
    } else if (isMulUnderflow(signedA, signedB)) {
        return { 0, ResultState::UNDERFLOW };
    } else {
        return { static_cast<std::uint64_t>(signedA * signedB), ResultState::OK };
    }
}

Result<std::uint64_t> mulUnsigned(std::uint64_t a, std::uint64_t b) {
    if (isMulOverflow(a, b)) {
        return { 0, ResultState::OVERFLOW };
    } else {
        return { a * b, ResultState::OK };
    }
}

Result<std::uint64_t> divSigned(std::uint64_t a, std::uint64_t b) {
    auto signedA = static_cast<std::int64_t>(a);
    auto signedB = static_cast<std::int64_t>(b);

    return { static_cast<std::uint64_t>(signedA / signedB), ResultState::OK };
}

Result<std::uint64_t> divUnsigned(std::uint64_t a, std::uint64_t b) { return { a / b, ResultState::OK }; }

Result<std::uint64_t> modSigned(std::uint64_t a, std::uint64_t b) {
    auto signedA = static_cast<std::int64_t>(a);
    auto signedB = static_cast<std::int64_t>(b);

    return { static_cast<std::uint64_t>(signedA % signedB), ResultState::OK };
}

Result<std::uint64_t> modUnsigned(std::uint64_t a, std::uint64_t b) { return { a % b, ResultState::OK }; }

bool canFitInt(Index type, std::uint64_t val) {
    assert(type::isIntType(type));

    using namespace pool::keys;

    const auto signedVal = static_cast<std::int64_t>(val);

    switch (type) {
    case I8_KEY_INDEX:
        return signedVal <= INT8_MAX && signedVal >= INT8_MIN;
    case U8_KEY_INDEX:
        return val <= UINT8_MAX;

    case I16_KEY_INDEX:
        return signedVal <= INT16_MAX && signedVal >= INT16_MIN;
    case U16_KEY_INDEX:
        return val <= UINT16_MAX;

    case I32_KEY_INDEX:
        return signedVal <= INT32_MAX && signedVal >= INT32_MIN;
    case U32_KEY_INDEX:
        return val <= UINT32_MAX;

    case I64_KEY_INDEX:
        return signedVal <= INT64_MAX && signedVal >= INT64_MIN;
    case U64_KEY_INDEX:
    case COMPTIME_INT_INDEX:
        return true;

    case ISIZE_KEY_INDEX:
    case USIZE_KEY_INDEX:
        // TODO:
        KOOLANG_TODO();
        return false;
    }

    KOOLANG_UNREACHABLE();
}
}
