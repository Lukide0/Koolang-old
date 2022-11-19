#ifndef KOOLANG_AIR_SYMBOL_NAMESPACE_H
#define KOOLANG_AIR_SYMBOL_NAMESPACE_H

#include "Record.h"
#include "util/Index.h"
#include <filesystem>
#include <unordered_map>

namespace air {

struct Module;

namespace symbol {
    struct Namespace {
        enum NamespaceType {
            ROOT,
            FILE,
            STRUCT,
            UNION,
        };

        Namespace(NamespaceType type, Module* mod)
            : Type(type)
            , Mod(mod) { }

        Index Parent = NULL_INDEX;
        Record* Rec  = nullptr;
        NamespaceType Type;
        Module* Mod;

        std::unordered_map<std::string, Index> SubNamespaces;
        std::unordered_map<std::string, Record*> Decls;
    };

}
}

#endif
