#ifndef KOOLANG_GLOBALS_H
#define KOOLANG_GLOBALS_H

#include <filesystem>
#include <string>
#include <vector>

namespace globals {

struct Config {
    enum Options {
        COLOR          = 1 << 0,
        ERROR_NORMAL   = 1 << 1,
        ERROR_JSON     = 1 << 2,
        ERROR_NONE     = 1 << 3,
        TARGET_X86_64  = 1 << 4,
        TARGET_X86     = 1 << 5,
        TEST           = 1 << 6,
        OPTIMAZE_0     = 1 << 7,
        OPTIMAZE_1     = 1 << 8,
        OPTIMAZE_2     = 1 << 9,
        OPTIMAZE_S     = 0 << 10,
        OPTIMAZE_RESET = OPTIMAZE_0 | OPTIMAZE_1 | OPTIMAZE_2 | OPTIMAZE_S,
        DEBUG_MODE     = 1 << 11,
    };

    enum class Command {
        NONE,
        BUILD_BIN,
        BUILD_LIB,
        BUILD_CLIB,
        BUILD_DYNLIB,
        BUILD_CDYNLIB,
        SHOW_KIR,
    };

    Options Flags   = static_cast<Options>(COLOR | ERROR_NORMAL | TARGET_X86_64 | OPTIMAZE_0);
    Command Command = Command::NONE;

    std::string OutputFile;
    std::string InputFile;
    std::vector<std::filesystem::path> ImportPaths;
    std::filesystem::path WorkingDir;
};

extern Config g_config;

}

#endif // KOOLANG_GLOBALS_H
