#include "air/ModuleManager.h"
#include "kir/Printer.h"
#include "terminal/globals.h"
#include "terminal/terminal.h"
#include <iostream>
#include <iterator>

constexpr int RET_OK  = 0;
constexpr int RET_ERR = 1;

int main(int argc, char* argv[]) {
    if (argc == 1) {
        terminal::printUsage();
        return RET_OK;
    }

    else if (!terminal::parseArgs(argc, argv, globals::g_config)) {
        return RET_ERR;
    }

    air::ModuleManager manager;

    switch (globals::g_config.Command) {
    case globals::Config::Command::SHOW_KIR: {
        // Manualy create module
        std::unique_ptr<air::Module> mod
            = std::make_unique<air::Module>(globals::g_config.InputFile, manager.Map, manager.InternPool);

        air::ModuleManager::ThreadContext context { mod.get(), nullptr };

        // fill the module field Kir
        air::ModuleManager::GenZirJob(context);

        kir::Printer(mod->Kir).Print();
        return RET_OK;
    }
    default:
        break;
    }

    air::Module* mainModule = manager.GenZir();

    if (mainModule->CompStatus == air::Module::Status::ERROR) {
        return RET_ERR;
    }

    manager.GenAir();

    // TODO:
    KOOLANG_TODO();

    return RET_OK;
}
