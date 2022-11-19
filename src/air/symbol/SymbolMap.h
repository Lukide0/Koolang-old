#ifndef KOOLANG_SYMBOL_SYMBOLMAP_H
#define KOOLANG_SYMBOL_SYMBOLMAP_H

#include "Namespace.h"
#include "Record.h"
#include "ast/Vis.h"
#include "util/Index.h"
#include <string_view>
#include <unordered_map>
#include <vector>

namespace air::symbol {

class SymbolMap {
public:
    SymbolMap();

    Record* CreateRecord(Index scope, std::string_view name, ast::Vis vis, Index kirInst, Index airInst, Module* mod);
    Index CreateNamespace(std::string_view name, Index parentScope, Module* mod, Namespace::NamespaceType type);

    [[nodiscard]] Module* GetModule(Index scope);
    [[nodiscard]] Record* GetRecord(Index record) { return m_records.at(record).get(); }
    [[nodiscard]] Namespace& GetNamespace(Index scope);

private:
    std::vector<Namespace> m_namespaces;
    std::vector<std::unique_ptr<Record>> m_records;
};
}

#endif
