#include "air/Module.h"
#include "air/Sema.h"

namespace air {

void Sema::KirDeclRef(Index inst) {
    const auto& data           = m_kir.Inst.at(inst).TokPl;
    const std::string& nameStr = m_kir.Strings.at(data.Payload.Offset);

    // We need to search only the top decls inside the module
    const auto scope = m_mod->Map.GetNamespace(m_mod->NamespaceIndex);
    const auto iter  = scope.Decls.find(nameStr);

    if (iter == scope.Decls.end()) {
        KOOLANG_ERR_MSG("UNKNOWN SYMBOL REFERENCE");
        return;
    }

    const symbol::Record* rec = iter->second;

    // check type circular dependency
    if (rec->StatusDecl == symbol::Record::State::IN_PROGRESS) {
        KOOLANG_ERR_MSG("CIRCULAR DEPENDENCY");
    } else if (rec->StatusDecl == symbol::Record::State::NOT_ANALYZED) {
        // cannot be nullptr because we have Record
        Sema* sema = rec->Mod->GetSema(rec->KirInst);
        sema->AnalyzeDecl();
    }

    if (rec->IsComptime) {
        CreateConstantFrom(inst, rec->Val);
    } else {
        CreateInst(InstType::SYMBOL, InstData::CreateSymbol(rec->Id, rec->Ty), inst);
    }
}

}
