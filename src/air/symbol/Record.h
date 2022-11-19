#ifndef KOOLANG_SYMBOL_RECORD_H
#define KOOLANG_SYMBOL_RECORD_H

#include "ast/Vis.h"
#include "util/Index.h"
#include <string>

namespace air {
struct Module;

namespace symbol {

    struct Record {
        Record() = default;
        Record(std::string_view name, ast::Vis isPub, Index kirInst, Index airInst, Module* mod, Index scope)
            : Name(name)
            , IsPub(isPub)
            , KirInst(kirInst)
            , AirInst(airInst)
            , Mod(mod)
            , Namespace(scope) { }

        enum class State {
            NOT_ANALYZED,
            IN_PROGRESS,
            COMPLETE,
        };

        Index Id;
        std::string_view Name;
        ast::Vis IsPub;

        Index Ty;
        Index Val;

        Index KirInst;
        Index AirInst;

        Module* Mod;
        Index Namespace;

        State StatusDecl = State::NOT_ANALYZED;
        State StatusBody = State::NOT_ANALYZED;

        bool IsComptime = true;
    };

}
}

#endif
