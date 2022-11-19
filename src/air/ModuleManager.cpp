#include "ModuleManager.h"
#include "Sema.h"
#include "ast/Parser.h"
#include "kir/AstGen.h"
#include "terminal/globals.h"
#include <filesystem>
#include <fstream>

namespace air {

ModuleManager::ModuleManager() {
    m_includePaths.push_back(globals::g_config.WorkingDir);
    m_includePaths.insert(
        m_includePaths.end(), globals::g_config.ImportPaths.begin(), globals::g_config.ImportPaths.end()
    );
}

Module* ModuleManager::CreateModuleWithNamespace(
    Index namespaceIndex, std::filesystem::path filepathWithoutExt, const std::filesystem::path& searchPath
) {
    namespace fs = std::filesystem;

    fs::path filepath;

    if (fs::is_directory(searchPath / filepathWithoutExt)) {
        filepath = filepathWithoutExt / "mod.k";
    } else {
        filepath = filepathWithoutExt.replace_extension(".k");
    }

    if (!fs::is_regular_file(searchPath / filepath)) {
        return nullptr;
    }

    Index currNamespace = namespaceIndex;
    fs::path currPath   = searchPath;
    Module* mod         = nullptr;

    for (const auto& part : filepath) {
        currPath /= part;

        auto& tmpNamespace = Map.GetNamespace(currNamespace);

        // module
        if (part == "mod.k") {
            mod = tmpNamespace.Mod;
            if (isNull(tmpNamespace.Mod)) {
                mod = m_modules.emplace_back(std::make_unique<Module>(currPath, Map, InternPool, currNamespace)).get();
                mod->FileData.Filepath = filepath.string();
                tmpNamespace.Mod       = mod;
            }
            break;
        }

        const std::string partStr(part.stem());
        const auto found = tmpNamespace.SubNamespaces.find(partStr);

        // already exists
        if (found != tmpNamespace.SubNamespaces.end()) {
            currNamespace = found->second;
            if (part.has_extension()) {
                mod = Map.GetModule(currNamespace);
            }
        } else {
            const Index parentNamespace = currNamespace;
            currNamespace = Map.CreateNamespace(partStr, parentNamespace, nullptr, symbol::Namespace::FILE);

            // file.k
            if (part.has_extension()) {
                mod = m_modules.emplace_back(std::make_unique<Module>(currPath, Map, InternPool, currNamespace)).get();
                mod->FileData.Filepath              = filepath.string();
                Map.GetNamespace(currNamespace).Mod = mod;
            }
        }
    }

    return mod;
}

Module* ModuleManager::GetOrAddFile(const std::string& filepathRaw, Index namespaceIndex) {
    namespace fs = std::filesystem;

    // this method is used in GenZirJob
    std::lock_guard<std::mutex> lock(m_mutex);

    const fs::path filepathWithoutExt(filepathRaw);

    Module* mod = nullptr;

    // search inside the given namespace
    const Module* parentMod = Map.GetModule(namespaceIndex);
    if (!isNull(parentMod)) {
        const auto& path = parentMod->SystemPath.parent_path();

        mod = CreateModuleWithNamespace(namespaceIndex, filepathWithoutExt, path);
    }

    if (isNull(mod)) {
        // search inside root namespaces
        for (const auto& path : m_includePaths) {
            mod = CreateModuleWithNamespace(NULL_INDEX, filepathWithoutExt, path);
            if (!isNull(mod)) {
                break;
            }
        }
    }

    // generate kir for the module
    if (!isNull(mod) && mod->CompStatus == Module::Status::NOT_LOADED) {
        mod->CompStatus = Module::Status::IN_PROGRESS;
        m_pool.Spawn(ModuleManager::GenZirJob, ThreadContext { mod, this });
    }

    return mod;
}

Module* ModuleManager::GenZir() {
    std::filesystem::path path(globals::g_config.InputFile);

    Module* mod = GetOrAddFile(path.stem());

    m_pool.Wait();

    return mod;
}

void ModuleManager::GenZirJob(ModuleManager::ThreadContext context) {
    Module* mod = context.Mod;

    std::fstream file(mod->SystemPath);
    mod->FileData.Content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    mod->FileData.State   = File::FileState::LOADED;
    file.close();

    {
        ast::Ast ast;
        {
            ast::Parser parser(mod->FileData, mod->SystemPath.string());
            ast = parser.Parse();

            if (!mod->FileData.ErrMsgs.empty()) {
                mod->CompStatus = Module::Status::ERROR;
                mod->FileData.PrintMsgs();
                return;
            }
        } // parser lifetime

        kir::AstGen gen(mod->Kir, ast);
        gen.Generate();

    } // ast lifetime

    mod->CompStatus = Module::Status::PREPARED;

    if (!mod->FileData.ErrMsgs.empty()) {
        mod->CompStatus = Module::Status::ERROR;
        mod->FileData.PrintMsgs();
        return;
    }

    // allows to parse file without following imported files
    if (isNull(context.Manager)) {
        return;
    }

    for (const auto strIndex : mod->Kir.Imports) {
        const std::string& path = mod->Kir.Strings.at(strIndex);
        Module* importMod       = context.Manager->GetOrAddFile(path, mod->NamespaceIndex);

        if (isNull(importMod)) {
            KOOLANG_ERR_MSG("Module not found \"{}\"", path);
            continue;
        } else if (importMod == mod) {
            KOOLANG_ERR_MSG("CANNOT IMPORT ITSELF: {}", path);
            continue;
        }

        mod->Imports.push_back(importMod);
    }

    Sema::PrepareModule(mod);
}

void ModuleManager::GenAir() {
    for (auto& mod : m_modules) {
        for (auto& sema : mod->Semas) {
            sema.Analyze();
        }
    }
}

}
