#ifndef KOOLANG_TERMINAL_TERMINAL_H
#define KOOLANG_TERMINAL_TERMINAL_H

#include "globals.h"

#include <filesystem>
#include <iostream>
#include <string_view>
#include <vector>

namespace terminal {

void init();

bool parseArgs(int argc, char** argv, globals::Config& config);
void printUsage();

}

#endif // KOOLANG_TERMINAL_TERMINAL_H
