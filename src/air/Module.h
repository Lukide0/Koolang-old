#ifndef KOOLANG_AIR_MODULE_H
#define KOOLANG_AIR_MODULE_H

#include "File.h"
#include "air/Inst.h"
#include "air/Sema.h"
#include "kir/Inst.h"
#include "symbol/SymbolMap.h"
#include "util/Index.h"
#include <algorithm>
#include <filesystem>
#include <map>
#include <unordered_map>
#include <utility>

namespace air {

struct Module {
    Module(std::filesystem::path systemPath, symbol::SymbolMap& map, Pool& pool, Index namespaceIndex = NULL_INDEX)
        : SystemPath(std::move(systemPath))
        , NamespaceIndex(namespaceIndex)
        , Map(map)
        , InternPool(pool) { }

    enum class Status {
        PREPARED,
        IN_PROGRESS,
        DONE,
        NOT_LOADED,
        NOT_EXISTS,
        ERROR,
    };

    Status CompStatus = Status::NOT_LOADED;
    std::filesystem::path SystemPath;
    std::filesystem::path* NamespacePath;

    File FileData;

    // if the file is mod then the index is the current module index otherwise it's the parent module index
    Index NamespaceIndex;
    symbol::SymbolMap& Map;
    Pool& InternPool;

    std::vector<Module*> Imports;

    std::vector<Air> Airs;
    std::vector<Sema> Semas;

    // TODO: maybe unique_ptr -> release when we have Air ?
    kir::Kir Kir;

    Sema* GetSema(Index kirInst) {
        Index leftBound = 0;
        auto rightBound = static_cast<Index>(Semas.size() - 1);

        while (leftBound <= rightBound) {
            const Index mid         = leftBound + (rightBound - leftBound) / 2;
            const Index semaKirInst = Semas.at(mid).GetKirInst();

            if (semaKirInst == kirInst) {
                return &Semas.at(mid);
            } else if (semaKirInst < kirInst) {
                leftBound = mid + 1;
            } else {
                rightBound = mid - 1;
            }
        }
        KOOLANG_WARN_MSG("{}", kirInst);
        return nullptr;
    }
};
}

#endif
