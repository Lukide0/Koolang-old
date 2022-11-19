#ifndef KOOLANG_LOGGER_RECORD_H
#define KOOLANG_LOGGER_RECORD_H

#include "Label.h"
#include <cstdint>
#include <string>
#include <vector>

struct File;

namespace logger {

enum class RecordKind {
    ERR,
    WARN,
    INFO,
};

class Record {
public:
    Record(RecordKind kind, const File* file, std::uint16_t code, std::string_view msg, const Label& label);
    void Print(std::ostream& stream);
    [[nodiscard]] RecordKind GetKind() const { return m_kind; }

private:
    RecordKind m_kind;
    std::uint16_t m_code;

    Label m_label;

    std::string_view m_msg;

    const File* m_file;
};

}

#endif
