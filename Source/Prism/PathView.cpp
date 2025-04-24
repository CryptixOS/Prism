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
    bool PathView::ValidateLength() { return m_Path.Size() < PRISM_MAX_PATH; }

    Vector<std::string> PathView::SplitPath()
    {
        Vector<std::string> segments;
        if (m_Path.Empty())
        {
            segments.PushBack("");
            return segments;
        }
        usize start     = m_Path[0] == '/' ? 1 : 0;
        usize end       = start;

        auto  findSlash = [this](usize pos) -> usize
        {
            usize current = pos;
            while (current < m_Path.Size() && m_Path[current] != '/') current++;

            return current == m_Path.Size() ? String::NPos : current;
        };

        while ((end = findSlash(start)) < m_Path.Size()
               && start < m_Path.Size())
        {
            StringView segment = m_Path.Substr(start, end - start);
            if (start != end)
                segments.EmplaceBack(segment.Raw(), segment.Size());

            start = end + 1;
        }

        // handle last segment
        if (start < m_Path.Size())
            segments.PushBack(m_Path.Substr(start).Raw());
        return std::move(segments);
    }

    StringView PathView::GetLastComponent() const
    {
        auto forthSlash = std::string_view(m_Path.Raw()).find_last_of('/');
        return m_Path.Substr(forthSlash, m_Path.Size() - forthSlash);
    }
}; // namespace Prism
