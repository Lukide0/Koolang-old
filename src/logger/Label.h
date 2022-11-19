#ifndef KOOLANG_LOGGER_LABEL_H
#define KOOLANG_LOGGER_LABEL_H

#include "Range.h"
#include "colors.h"
#include <string>
namespace logger {

struct Label {
    Label() = default;
    Label(std::string_view msg, Range range);
    Label& WithColor(Color color);

    std::string Msg;

    Color Fg = DEFAULT_COLOR;
    Range StrRange;
};

}
#endif
