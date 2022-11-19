#ifndef KOOLANG_UTIL_CACHE_H
#define KOOLANG_UTIL_CACHE_H

#include "util/Index.h"
class Cache {
public:
    [[nodiscard]] Index Size() const { return static_cast<Index>(m_data.size()); }
    void Add(Index value) { m_data.push_back(value); }

    // Removes the inserted elements.
    // returns the index from where the values were inserted
    Index InsertInto(std::vector<Index>& into, Index cacheBeginIndex) {
        const auto index = static_cast<Index>(into.size());
        into.insert(into.end(), m_data.begin() + cacheBeginIndex, m_data.end());

        RemoveAll(cacheBeginIndex);

        return index;
    }

    void RemoveAll(Index begin) { m_data.resize(begin); }

    void Set(Index index, Index value) { m_data[index] = value; }

private:
    std::vector<Index> m_data;
};

#endif
