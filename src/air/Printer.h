#ifndef KOOLANG_AIR_PRINTER_H
#define KOOLANG_AIR_PRINTER_H

#include "Sema.h"
#include "util/Index.h"
#include <sstream>
namespace air {

class Printer {
public:
    Printer(const Sema* sema);
    void Print();

private:
    void WriteInst(Index ref);
    void WriteConstant(Index inst);
    void WriteSymbol(Index inst);
    void WriteInternPoolData(Index index);
    void WriteKnownInternPoolKey(Index index);

    void WritePoolInt(const Pool& pool, Index extra);
    void WritePoolTypeValue(const Pool& pool, Index extra);
    void WriteBinOp(Index inst, std::string_view name);
    std::stringstream m_buffer;
    const Sema* m_sema;
    const Air& m_air;
    std::size_t m_offset = 0;
};

}

#endif
