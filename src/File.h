#ifndef KOOLANG_FILE_H
#define KOOLANG_FILE_H

#include "logger/Record.h"
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

// forward declaration
struct File

{
    enum class FileState {
        NOT_LOADED,
        LOADED,
    };

    std::string Filepath;
    std::string Content;
    FileState State = FileState::NOT_LOADED;
    std::vector<logger::Record> ErrMsgs;
    std::vector<logger::Record> WarnMsgs;
    std::vector<logger::Record> OtherMsgs;

    void AddMsg(logger::Record&& record) {
        switch (record.GetKind()) {
        case logger::RecordKind::ERR:
            ErrMsgs.push_back(record);
            break;
        case logger::RecordKind::WARN:
            WarnMsgs.push_back(record);
            break;
        case logger::RecordKind::INFO:
            OtherMsgs.push_back(record);
            break;
        }
    }

    void PrintMsgs() {
        for (auto& msg : ErrMsgs) {
            msg.Print(std::cerr);
        }

        for (auto& msg : WarnMsgs) {
            msg.Print(std::cerr);
        }

        for (auto& msg : OtherMsgs) {
            msg.Print(std::cout);
        }
    }
};

#endif
