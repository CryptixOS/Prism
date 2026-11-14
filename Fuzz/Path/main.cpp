/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
// Paths from the fuzzer through every PathView and Path operation.
#include <Prism/Core/Types.hpp>
import Prism.Utility.Path;
import Prism.Utility.PathView;

using namespace Prism;

extern "C" int LLVMFuzzerTestOneInput(const u8* data, usize size)
{
    StringView text(reinterpret_cast<const char*>(data), size);
    PathView   view(text);

    (void)view.Absolute();
    (void)view.BaseName();
    (void)view.ParentName();
    (void)view.ParentPath();
    (void)view.Extension();
    (void)view.ValidateLength();
    for (const String& part : view.Split()) (void)part.Size();

    Path path(text);
    (void)path.Size();
    return 0;
}
