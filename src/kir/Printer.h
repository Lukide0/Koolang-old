#ifndef KOOLANG_KIR_PRINTER_H
#define KOOLANG_KIR_PRINTER_H

#include "Inst.h"
#include <sstream>
namespace kir {

class Printer {
public:
    Printer(const Kir& kir);
    void Print();

private:
    void WriteInst(Index ref);
    void WriteStr(Index str);
    void WriteBlockInline(Index block);
    void WriteBlock(Index block);
    void WriteBlockRaw(Index block);

    void WriteBin(std::string_view name, Index bin);
    void WriteSingleOpInst(std::string_view name, Index nodepl);
    void WriteCond(std::string_view text, Index extra);
    void WriteInstRef(RefInst ref);
    void Write(std::string_view text);
    void Write(char character);
    void WriteNewLine();
    void WriteRange(Index start, Index count);

    void WriteDecl(Index ref);
    void WriteDeclFn(Index ref);
    void WriteDeclTrait(Index ref);
    void WriteDeclImpl(Index ref);
    void WriteBinInst(std::string_view name, Index nodepl);
    void WriteDeclRef(Index ref);
    void WriteDeclItem(Index ref);
    void WriteNamespace(Index ref);
    void WriteStructInit(Index ref);
    void WriteFunc(Index ref);
    void WriteParam(Index ref);
    void WriteIf(Index ref);

    std::stringstream m_buffer;
    const Kir& m_kir;
    std::size_t m_offset = 0;
};

}

#endif
