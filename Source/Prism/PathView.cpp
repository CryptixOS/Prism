/*
 * Created by v1tr10l7 on 23.01.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/PathView.hpp>

#if PRISM_TARGET_CRYPTIX == 1
    #include <API/Limits.hpp>
    #define PRISM_MAX_PATH Limits::MAX_PATH_LENGTH
#else
    #define PRISM_MAX_PATH 255
#endif

namespace Prism
{
    bool PathView::ValidateLength() { return m_Path.size() < PRISM_MAX_PATH; }

    Vector<std::string> PathView::SplitPath()
    {
        Vector<std::string> segments;
        usize               start     = m_Path[0] == '/' ? 1 : 0;
        usize               end       = start;

        auto                findSlash = [this](usize pos) -> usize
        {
            usize current = pos;
            while (m_Path[current] != '/' && current < m_Path.size()) current++;

            return current == m_Path.size() ? std::string::npos : current;
        };

        std::string path = m_Path.data();
        while ((end = findSlash(start)) != std::string::npos)
        {
            std::string segment = path.substr(start, end - start);
            if (start != end) segments.PushBack(segment);

            start = end + 1;
        }

        // handle last segment
        if (start < path.length()) segments.PushBack(path.substr(start));
        return std::move(segments);
    }

    std::string_view PathView::GetLastComponent() const
    {
        auto forthSlash = m_Path.find_last_of('/');
        return m_Path.substr(forthSlash, m_Path.size() - forthSlash);
    }
}; // namespace Prism
