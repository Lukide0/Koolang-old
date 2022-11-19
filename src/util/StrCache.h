#ifndef KOOLANG_UTIL_STRCACHE_H
#define KOOLANG_UTIL_STRCACHE_H

#include "util/Index.h"
#include "util/string_hash.h"
class StrCache {
public:
    StrCache(std::vector<std::string>& strings)
        : m_strings(strings) { }

    Index GetOrCreateStr(std::string_view str) {
        const auto iter = m_map.find(str);
        if (iter != m_map.end()) {
            return iter->second;
        }

        const auto index = static_cast<Index>(m_strings.size());

        m_strings.emplace_back(str);
        m_map.emplace(str, index);

        return index;
    }

private:
    std::unordered_map<std::string_view, Index, string_hash> m_map;
    std::vector<std::string>& m_strings;
};

#endif
