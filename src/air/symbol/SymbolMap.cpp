#include "SymbolMap.h"
#include "air/Module.h"
#include <memory>

namespace air::symbol {

SymbolMap::SymbolMap() {
    m_records.emplace_back();
    m_namespaces.emplace_back(Namespace::ROOT, nullptr);
}

Record*
SymbolMap::CreateRecord(Index scope, std::string_view name, ast::Vis vis, Index kirInst, Index airInst, Module* mod) {
    Record* rec = m_records.emplace_back(std::make_unique<Record>(name, vis, kirInst, airInst, mod, scope)).get();
    rec->Id     = static_cast<Index>(m_records.size() - 1);

    m_namespaces[scope].Decls.emplace(name, rec);
    return rec;
}

Index SymbolMap::CreateNamespace(std::string_view name, Index parentScope, Module* mod, Namespace::NamespaceType type) {
    const auto index = static_cast<Index>(m_namespaces.size());

    m_namespaces.emplace_back(type, mod);
    m_namespaces[parentScope].SubNamespaces.emplace(name, index);

    return index;
}

Module* SymbolMap::GetModule(Index scope) { return m_namespaces.at(scope).Mod; }
Namespace& SymbolMap::GetNamespace(Index scope) { return m_namespaces.at(scope); }

}
