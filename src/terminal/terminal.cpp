#include "terminal.h"
#include "globals.h"

#include "config.h"

#ifdef _WIN32
#include <windows.h>
#endif

namespace terminal {

void init() {
#ifdef _WIN32
    SetConsoleCP(65001);
#endif
}

bool parseArgs(int argc, char** argv, globals::Config& config) {
    using globals::Config;

    for (int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];
        if (arg == "--help") {
            printUsage();
            return false;
        }

        if (arg == "--version" || arg == "-v") {
            std::cout << "koolang version " << VERSION << std::endl;
            return false;
        }

        if (arg == "--test") {
            config.Flags = static_cast<Config::Options>(config.Flags | Config::TEST);
        } else if (arg == "-d" || arg == "--debug") {
            config.Flags = static_cast<Config::Options>(config.Flags | Config::DEBUG_MODE);
        } else if (arg == "--target") {
            if (i + 1 == argc) {
                std::cout << "ERROR: Expected value after --target" << std::endl;
                return false;
            }
            arg = argv[++i];
            if (arg == "x86") {
                config.Flags = static_cast<Config::Options>(config.Flags | Config::TARGET_X86);
            } else if (arg == "x86_64") {
                config.Flags = static_cast<Config::Options>(config.Flags | Config::TARGET_X86_64);
            } else {
                std::cout << "ERROR: Invalid value for --target" << std::endl;
                return false;
            }
        } else if (arg == "--optimaze") {
            if (i + 1 == argc) {
                std::cout << "ERROR: Expected value after --optimaze" << std::endl;
                return false;
            }
            arg = argv[++i];
            if (arg == "0") {
                config.Flags
                    = static_cast<Config::Options>((config.Flags ^ Config::OPTIMAZE_RESET) | Config::OPTIMAZE_0);
            } else if (arg == "1") {
                config.Flags
                    = static_cast<Config::Options>((config.Flags ^ Config::OPTIMAZE_RESET) | Config::OPTIMAZE_1);
            } else if (arg == "2") {
                config.Flags
                    = static_cast<Config::Options>((config.Flags ^ Config::OPTIMAZE_RESET) | Config::OPTIMAZE_2);
            } else if (arg == "s") {
                config.Flags
                    = static_cast<Config::Options>((config.Flags ^ Config::OPTIMAZE_RESET) | Config::OPTIMAZE_S);
            } else {
                std::cout << "ERROR: Invalid value for --optimaze" << std::endl;
                return false;
            }
        } else if (arg == "build") {
            if (config.Command != Config::Command::NONE) {
                // TOOD: better message
                std::cout << "error: command is already set" << std::endl;
                return false;
            }

            if (i + 1 == argc) {
                std::cout << "error: expected value after --build" << std::endl;
                return false;
            }
            arg = argv[++i];
            if (arg == "bin") {
                config.Command = Config::Command::BUILD_BIN;
            } else if (arg == "lib") {
                config.Command = Config::Command::BUILD_LIB;
            } else if (arg == "clib") {
                config.Command = Config::Command::BUILD_CLIB;
            } else if (arg == "dylib") {
                config.Command = Config::Command::BUILD_DYNLIB;
            } else if (arg == "cdylib") {
                config.Command = Config::Command::BUILD_CDYNLIB;
            } else {
                std::cout << "ERROR: Invalid value for build" << std::endl;
                return false;
            }
        } else if (arg == "-I") {
            if (i + 1 == argc) {
                std::cout << "ERROR: Expected value after -I" << std::endl;
                return false;
            }

            arg = argv[++i];

            std::filesystem::path dir(arg);

            if (!std::filesystem::is_directory(dir)) {
                std::cout << "ERROR: Import path doesn't exist '" << dir << '\'' << std::endl;
                return false;
            }

            config.ImportPaths.emplace_back(dir);
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 == argc) {
                std::cout << "ERROR: Expected value after --output" << std::endl;
                return false;
            }
            arg               = argv[++i];
            config.OutputFile = arg;
        } else if (arg == "kir") {

            if (config.Command != Config::Command::NONE) {
                // TOOD: better message
                std::cout << "error: command is already set" << std::endl;
                return false;
            }
            config.Command = Config::Command::SHOW_KIR;
        } else {
            if (i + 1 == argc) {
                std::filesystem::path path = std::filesystem::absolute(arg);
                if (!std::filesystem::exists(path)) {
                    std::cout << "ERROR: File '" << path << "' not found" << std::endl;
                    return false;
                }

                config.InputFile  = arg;
                config.WorkingDir = path.parent_path();
            } else {
                std::cout << "ERROR: Invalid flag '" << arg << '\'' << std::endl;
                return false;
            }
        }
    }

    if (config.InputFile.empty()) {
        std::cout << "ERROR: Missing input file" << std::endl;
        return false;
    }

    if (config.Command == Config::Command::NONE) {
        std::cout << "ERROR: Missing command" << std::endl;
        return false;
    }

    return true;
}

void printUsage() {
    std::cout << "Usage: koolang [build|kir] [options] <file>\n"
              << "\n"
              << "Options:\n"
              << "  --help            get help\n"
              << "  -v --version      print a version\n"
              << "  --target          specify target architecture\n"
              << "      x86_64        [default]\n"
              << "      x86\n"
              << "  --test            run tests\n"
              << "  -o --output       filename of the output\n"
              << "  --optimaze\n"
              << "      0             no optimazations [default]\n"
              << "      1             basic optimazations\n"
              << "      2             all optimazations\n"
              << "      s             optimaze for binary size\n"
              << "  -d --debug        include debug information\n"
              << "  -I                add import path\n"
              << "Commands:"
              << "  build             specify what compiler emits\n"
              << "      bin           [default]\n"
              << "      lib           koolang library\n"
              << "      clib          c library\n"
              << "      dylib         koolang dynamic library\n"
              << "      cdylib        c dynamic library\n"
              << "  kir               prints KIR\n";
}

}
