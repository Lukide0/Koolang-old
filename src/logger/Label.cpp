#include "Label.h"

namespace logger {

Label::Label(std::string_view msg, Range range)
    : Msg(msg)
    , StrRange(range) { }

Label& Label::WithColor(Color color) {
    Fg = color;
    return *this;
}

}
