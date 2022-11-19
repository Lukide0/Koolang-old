#ifndef KOOLANG_AST_VIS_H
#define KOOLANG_AST_VIS_H

#include <cstdint>
namespace ast {

enum class Vis : std::uint8_t {
    LOCAL,
    GLOBAL,
};

}

#endif
