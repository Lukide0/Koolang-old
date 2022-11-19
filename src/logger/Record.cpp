#include "Record.h"
#include "File.h"
#include "Label.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

namespace logger {

struct LineInfo {
    std::size_t End;
    std::size_t Start;

    int Num;
};

LineInfo getLineInfo(std::size_t pos, const File& file) {
    LineInfo info;

    info.End   = file.Content.find('\n', pos);
    info.Start = file.Content.rfind('\n', pos);

    if (info.Start == std::string::npos) {
        info.Start = 0;
    } else {
        info.Start += 1;
    }

    info.Num = std::accumulate(
        file.Content.begin(), std::next(file.Content.begin(), static_cast<int>(info.Start)), 1,
        [](int prev, char c) { return prev + static_cast<int>(c == '\n'); }
    );

    return info;
}

std::string colorStr(Color color, const std::string& str) { return color.ToString() + str + Color::RESET; }

Record::Record(RecordKind kind, const File* file, std::uint16_t code, std::string_view msg, const Label& label)
    : m_kind(kind)
    , m_code(code)
    , m_label(label)
    , m_msg(msg)
    , m_file(file) { }

void Record::Print(std::ostream& stream) {
    /*
    TODO:
        - multiple labels
        - multiple lines
        - multiple files
        - example: https://docs.rs/ariadne/latest/ariadne/index.html
    */

    std::stringstream ss;

    Color titleColor = Color::DEFAULT;

    switch (m_kind) {
    case RecordKind::ERR:
        titleColor = Color::RED;
        ss << titleColor.ToString() << "ERR[E";
        break;
    case RecordKind::WARN:
        titleColor = Color::YELLOW;
        ss << titleColor.ToString() << "WARN[W";
        break;
    case RecordKind::INFO:
        titleColor = Color::CYAN;
        ss << titleColor.ToString() << "INFO[I";
        break;
    }

    // format code, e.g. 1 -> 001
    ss << std::setfill('0') << std::setw(3) << m_code;
    // record message
    ss << "]: " << Color::RESET << m_msg << '\n';

    LineInfo lineInfo = getLineInfo(m_label.StrRange.Start, *m_file);

    // calculate padding (count of digits)
    int padding = static_cast<int>(log10(lineInfo.Num)) + 1;

    std::string padStr(static_cast<unsigned int>(padding), ' ');

    //-- source --//
    ss << padStr << colorStr(Color::BRIGHT_BLACK, " ╭─[") << m_file->Filepath << colorStr(Color::BRIGHT_BLACK, "]\n");
    ss << padStr << colorStr(Color::BRIGHT_BLACK, " │\n");

    //-- label --//

    // line number
    ss << Color(Color::BRIGHT_BLACK).ToString() << std::setw(padding) << std::setfill(' ') << lineInfo.Num << " │ "
       << Color::RESET;

    std::string lineContent = m_file->Content.substr(lineInfo.Start, lineInfo.End - lineInfo.Start);

    std::size_t relativeStart = m_label.StrRange.Start - lineInfo.Start;
    std::size_t relativeEnd   = m_label.StrRange.End - lineInfo.Start;
    std::size_t strLen        = relativeEnd - relativeStart;

    // before label code
    ss << lineContent.substr(0, relativeStart);

    // label code
    ss << m_label.Fg.ToString() << lineContent.substr(relativeStart, strLen) << Color::RESET;

    // after label code
    ss << lineContent.substr(relativeEnd) << '\n';

    ss << Color(Color::BRIGHT_BLACK).ToString() << padStr << " │\n";
    // label message
    ss << padStr << " ╰┤ " << Color::RESET << m_label.Msg << '\n';

    stream << ss.str();
}

}
