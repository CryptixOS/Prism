/*
 * Created by v1tr10l7 on 21.01.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Utility/Path.hpp>

namespace Prism
{
    bool           Path::ValidateLength() { return View().ValidateLength(); }
    Vector<String> Path::Split(ValueType delimiter) const
    {
        return View().Split(delimiter);
    }
    StringView Path::BaseName() const { return PathView(*this).BaseName(); }
    StringView Path::ParentName() const { return PathView(*this).ParentName(); }
    Path       Path::ParentPath() const
    {
        auto view = PathView(m_Path);
        return view.ParentPath();
    }
    StringView Path::Extension() const { return PathView(*this).Extension(); }
}; // namespace Prism
