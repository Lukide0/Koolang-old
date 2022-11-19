#ifndef KOOLANG_AIR_MODULEMANAGER_H
#define KOOLANG_AIR_MODULEMANAGER_H

#include "Module.h"
#include "Pool.h"
#include "symbol/SymbolMap.h"
#include "util/Index.h"
#include "util/ThreadPool.h"
#include <mutex>
#include <string_view>

namespace air {

class ModuleManager {
public:
    struct ThreadContext {
        Module* Mod;
        ModuleManager* Manager;
    };
    symbol::SymbolMap Map;
    Pool InternPool;

    ModuleManager();

    Module* GetOrAddFile(const std::string& filepathRaw, Index namespaceIndex = NULL_INDEX);
    Module* AddFileSingle(const std::string& filepathRaw, Index namespaceIndex = NULL_INDEX);

    Module* GenZir();

    void GenAir();

    static void GenZirJob(ThreadContext context);

private:
    Module* CreateModuleWithNamespace(
        Index namespaceIndex, std::filesystem::path filepathWithoutExt, const std::filesystem::path& searchPath
    );

    std::vector<std::unique_ptr<Module>> m_modules;
    std::vector<std::filesystem::path> m_includePaths;

    std::mutex m_mutex;
    ThreadPool<ThreadContext> m_pool;
};

}

#endif
