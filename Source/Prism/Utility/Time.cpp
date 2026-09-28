/*
 * Created by v1tr10l7 on 26.09.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/String/StringBuilder.hpp>
#include <Prism/Utility/Time.hpp>

namespace Prism::Time
{
    static inline void FormatTwoDigits(StringBuilder<char>& builder, u64 val)
    {
        if (val < 10) { builder << '0'; }
        builder << val;
    }

    BasicString<char, CharTraits<char>>
    Timestep::ToUtcDateString(const Timestep& timestep)
    {
        u64 totalSeconds = timestep.m_NanoSeconds / 1000000000ull;

        u64 sec          = totalSeconds % 60;
        u64 totalMinutes = totalSeconds / 60;
        u64 min          = totalMinutes % 60;
        u64 totalHours   = totalMinutes / 60;

        u64 hour         = totalHours % 24;

        u64 days         = totalHours / 24;

        days += 719468;
        u64 era = days / 146097;
        u64 doe = days % 146097;
        u64 yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
        u64 y   = yoe + era * 400;
        u64 doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
        u64 mp  = (5 * doy + 2) / 153;
        u64 d   = doy - (153 * mp + 2) / 5 + 1;
        u64 m   = mp + (mp < 10 ? 3 : -9);
        y += (m <= 2);

        StringBuilder builder(20);

        builder << y << '-';
        FormatTwoDigits(builder, m);
        builder << '-';
        FormatTwoDigits(builder, d);
        builder << ' ';

        FormatTwoDigits(builder, hour);
        builder << ':';
        FormatTwoDigits(builder, min);
        builder << ':';
        FormatTwoDigits(builder, sec);

        return builder.ToString();
    }

    BasicString<char, CharTraits<char>>
    Timestep::ToTimeString(const Timestep& timestep)
    {
        u64 totalSeconds = timestep.m_NanoSeconds / 1000000000ull;

        u64 sec          = totalSeconds % 60;
        u64 totalMinutes = totalSeconds / 60;
        u64 min          = totalMinutes % 60;
        u64 totalHours   = totalMinutes / 60;
        u64 hour         = totalHours % 24;

        // FIXME(v1tr10l7): temporary fix, handle local utc offset
        hour += 2;

        StringBuilder builder(8);

        FormatTwoDigits(builder, hour);
        builder << ':';
        FormatTwoDigits(builder, min);
        builder << ':';
        FormatTwoDigits(builder, sec);

        return builder.ToString();
    }
}; // namespace Prism::Time
