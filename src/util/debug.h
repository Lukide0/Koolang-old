#ifndef KOOLANG_UTIL_DEBUG_H
#define KOOLANG_UTIL_DEBUG_H

#include "logger/colors.h"
#include <format>
#include <iostream>
#include <utility>

#ifdef KOOLANG_DEBUG_MODE
#define KOOLANG_DEBUG true
#define KOOLANG_MSG(TITLE, MSG, COLOR) \
    std::cout << logger::Color(COLOR).ToString() << TITLE << ": " << logger::Color::RESET << MSG << '\n'
#else
#define KOOLANG_DEBUG false
#define KOOLANG_MSG(TITLE, MSG, STYLE)
#endif

#define STRINGIZE_UTIL(X) #X
#define STRINGIZE(X) STRINGIZE_UTIL(X)
#define LINE_STR STRINGIZE(__LINE__)

// clang-format off
#define KOOLANG_DEBUG_MSG(...) KOOLANG_MSG("[DEBUG](" __FILE__ ":" LINE_STR ")", std::format(__VA_ARGS__), logger::Color::BRIGHT_GREEN)
#define KOOLANG_ERR_MSG(...)   KOOLANG_MSG("[ERR](" __FILE__ ":" LINE_STR ")", std::format(__VA_ARGS__), logger::Color::RED)
#define KOOLANG_WARN_MSG(...)  KOOLANG_MSG("[WARN](" __FILE__ ":" LINE_STR ")", std::format(__VA_ARGS__), logger::Color::YELLOW)
#define KOOLANG_INFO_MSG(...)  KOOLANG_MSG("[INFO](" __FILE__ ":" LINE_STR ")", std::format(__VA_ARGS__), logger::Color::CYAN)
#define KOOLANG_FATAL_MSG(...) KOOLANG_MSG("[FATAL](" __FILE__ ":" LINE_STR ")", std::format(__VA_ARGS__), logger::Color::BRIGHT_MAGENTA)

#define KOOLANG_CUSTOM_MSG(TITLE, ...) KOOLANG_MSG(TITLE, std::format(__VA_ARGS__), logger::Color::GREEN)
#define KOOLANG_TODO() KOOLANG_MSG("[TODO](" __FILE__ ":" LINE_STR ")", "This feature is not implemented yet", logger::Color::YELLOW)
#define KOOLANG_UNREACHABLE() KOOLANG_FATAL_MSG("Something went wrong!"); std::unreachable()
#define KOOLANG_TODO_EXIT(...) KOOLANG_FATAL_MSG(__VA_ARGS__); std::exit(-1)

// clang-format on

#endif
