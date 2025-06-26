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

    StringView PathView::GetLastComponent() const
    {
        auto forthSlash = m_Path.FindLastOf('/');
        if (forthSlash > m_Path.Size()) return m_Path;

        return m_Path.Substr(forthSlash);
    }
}; // namespace Prism
