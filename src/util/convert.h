#ifndef KOOLANG_UTIL_CONVERT_H
#define KOOLANG_UTIL_CONVERT_H

#include "util/debug.h"
#include <cmath>
#include <cstdint>
#include <string>
#include <string_view>

std::uint64_t convertToU64(std::string_view str, bool& err) {
    // overflow tests
    constexpr std::uint64_t TEST_HEX_BITS_MASK = static_cast<std::uint64_t>(0xF) << 59;
    constexpr std::uint64_t TEST_OCT_BITS_MASK = static_cast<std::uint64_t>(7) << 59;
    constexpr std::uint64_t TEST_BIN_BITS_MASK = static_cast<std::uint64_t>(0x1) << 63;
    constexpr auto U64_MAX                     = ~static_cast<std::uint64_t>(0);
    constexpr std::uint64_t DEC_MAX_VAL        = U64_MAX / 10;

    err = false;

    std::uint64_t result = 0;

    if (str[0] == '0') {
        if (str.length() == 1) {
            return result;
        }

        switch (str[1]) {
        // hexadecimal number
        case 'x':
            for (const auto digit : str.substr(2)) {
                if (digit == '_') {
                    continue;
                }

                // Check if fits
                if ((result & TEST_HEX_BITS_MASK) != 0) {
                    err = true;
                    return 0;
                }

                result <<= 4;

                if (digit >= '0' && digit <= '9') {
                    result += static_cast<std::uint8_t>(digit - '0');
                }
                // A - F
                else {
                    result += static_cast<std::uint8_t>(digit - 'A') + 10;
                }
            }
            return result;
        case 'o':
            for (const auto digit : str.substr(2)) {
                if (digit == '_') {
                    continue;
                }
                // Check if fits
                if ((result & TEST_OCT_BITS_MASK) != 0) {
                    err = true;
                    return 0;
                }

                result <<= 3;
                result += static_cast<std::uint8_t>(digit - '0');
            }
            return result;
        case 'b':
            for (const auto digit : str.substr(2)) {
                if (digit == '_') {
                    continue;
                }

                // Check if fits
                if ((result & TEST_BIN_BITS_MASK) != 0) {
                    err = true;
                    return 0;
                }

                result <<= 1;
                result += static_cast<std::uint8_t>(digit - '0');
            }
            return result;
        default:
            break;
        }
    }

    for (const auto digit : str) {
        if (digit == '_') {
            continue;
        }

        const auto val = static_cast<std::uint8_t>(digit - '0');

        // If the result value will be greater than 64bit int
        if (result > DEC_MAX_VAL || (result == DEC_MAX_VAL && val > (U64_MAX % 10))) {
            err = true;
            return 0;
        }

        result *= 10;
        result += val;
    }

    return result;
}

double convertToF64(std::string_view str, bool& err) {
    const auto pos = str.find_first_of('.');
    // TODO: REMAKE -> CURRENT MAX VALUE IS U64_MAX < F64_MAX
    KOOLANG_TODO();

    // XX.0
    auto partBefore = static_cast<double>(convertToU64(str.substr(0, pos), err));

    if (err) {
        return 0;
    }

    const auto afterStr = str.substr(pos + 1);

    int zeros = 0;
    for (const auto digit : afterStr) {
        if (digit >= '1' && digit <= '9') {
            break;
        } else if (digit == '_') {
            continue;
        } else {
            zeros += 1;
        }
    }

    std::uint64_t afterFP = convertToU64(afterStr, err);

    if (err) {
        return 0;
    }

    double digits = 1 + zeros;

    if (afterFP != 0) {
        digits += std::floor(std::log10(afterFP));
    }

    // 0.XX
    double partAfter = static_cast<double>(afterFP) / std::pow(10, digits);

    // Check overflow
    if (partAfter > std::numeric_limits<double>::max() - partBefore) {
        err = true;
        return 0;
    } else {
        return partBefore + partAfter;
    }
}

#endif
