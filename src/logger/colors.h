#ifndef KOOLANG_LOGGER_COLORS_H
#define KOOLANG_LOGGER_COLORS_H

#include <cstdint>
#include <string>
namespace logger {

struct Color {
    static constexpr std::string RESET = "\x1B[0m";

    enum ColorCode {
        BLACK   = 30,
        RED     = 31,
        GREEN   = 32,
        YELLOW  = 33,
        BLUE    = 34,
        MAGENTA = 35,
        CYAN    = 36,
        WHITE   = 37,
        DEFAULT = 39,

        BRIGHT_BLACK   = 90,
        BRIGHT_RED     = 91,
        BRIGHT_GREEN   = 92,
        BRIGHT_YELLOW  = 93,
        BRIGHT_BLUE    = 94,
        BRIGHT_MAGENTA = 95,
        BRIGHT_CYAN    = 96,
        BRIGHT_WHITE   = 97,
    };

    constexpr Color(ColorCode color)
        : Code(color) { }

    ColorCode Code;

    [[nodiscard]] std::string ToString(bool background = false) const;
};

constexpr auto DEFAULT_COLOR = Color(Color::DEFAULT);

}

#endif
