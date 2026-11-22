/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
import Prism.Core.Version;
import Prism.Test.Test;

using namespace Prism;

TEST_CASE(version_matches_the_build)
{
    EXPECT_EQ(VERSION.Major, 0);
    EXPECT_EQ(VERSION.Minor, 2);
    EXPECT_EQ(VERSION.Patch, 0);
    EXPECT(LIBRARY_NAME == "prism");
    EXPECT(VERSION_STRING.StartsWith("0.2.0-"));
    EXPECT(Format("{}", VERSION) == "0.2.0");
    EXPECT(static_cast<String>(VERSION) == "0.2.0");
}

TEST_CASE(versions_order)
{
    EXPECT(u64(Version(0, 2, 0)) > u64(Version(0, 1, 9)));
    EXPECT(u64(Version(1, 0, 0)) > u64(Version(0, 9, 9)));
    EXPECT(u64(Version(0, 2, 1)) > u64(Version(0, 2, 0)));
}
