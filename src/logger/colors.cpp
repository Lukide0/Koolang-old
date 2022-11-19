#include "colors.h"

namespace logger {

std::string Color::ToString(bool background) const {
    std::string colorStr = "\x1B[";

    auto code = static_cast<int>(Code);

    if (background) {
        code += 10;
    }

    colorStr += std::to_string(code) + 'm';
    return colorStr;
}

}
