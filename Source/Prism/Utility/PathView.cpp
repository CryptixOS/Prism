/*
 * Created by v1tr10l7 on 23.01.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Utility/PathView.hpp>

#if PRISM_TARGET_CRYPTIX == 1
    #include <API/Limits.hpp>
    #define PRISM_MAX_PATH Limits::MAX_PATH_LENGTH
#else
    #define PRISM_MAX_PATH 255
#endif

namespace Prism
{
    bool PathView::ValidateLength() { return m_Path.Size() < PRISM_MAX_PATH; }

    StringView PathView::BaseName() const
    {
        if (m_Path.Empty()) return ""_sv;

        usize end = m_Path.FindLastNotOf('/');
        // path was only slashes
        if (end == StringView::NPos) return {};

        usize start = m_Path.FindLastOf('/', end);
        if (start == StringView::NPos) return Substr(0, end + 1);

        return Substr(start + 1, end - start);
    }
    StringView PathView::ParentName() const
    {
        if (m_Path.Empty()) return ""_sv;

        usize end = FindLastNotOf('/');
        if (end == NPos) return ""_sv;

        usize lastSlash = FindLastOf('/', end);
        // no parent
        if (lastSlash == NPos) return ""_sv;

        usize parentEnd = FindLastNotOf('/', lastSlash);
        if (parentEnd == NPos) return ""_sv;

        usize parentStart = FindLastOf('/', parentEnd);
        if (parentStart == NPos) parentStart = 0;
        else parentStart++;

        return Substr(parentStart, parentEnd - parentStart + 1);
    }
    PathView PathView::ParentPath() const
    {
        if (Empty()) return ""_pv;

        usize end = FindLastNotOf('/');
        if (end == NPos) return ""_pv;

        usize slash = FindLastOf('/', end);
        if (slash == NPos) return ""_pv;

        return Substr(0, slash);
    }
    StringView PathView::Extension() const
    {
        auto  base = BaseName();
        usize dot  = base.FindLastOf('.');

        // no extension or hidden file
        if (dot == NPos || dot == 0 || dot == base.Size() - 1) return ""_sv;

        return Substr(dot + 1);
    }
}; // namespace Prism
