/*
 * Created by v1tr10l7 on 21.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism::Time
{
    constexpr usize SECONDS_PER_MINUTE = 60;
    constexpr usize SECONDS_PER_HOUR   = 3600;
    constexpr usize SECONDS_PER_DAY    = 86400;

    constexpr bool  IsLeapYear(u64 year)
    {
        return ((year % 4 == 0) && ((year % 100 != 0) || (year % 400) == 0));
    }
    constexpr u64 DaysInMonth(u8 month, u64 year)
    {
        constexpr u32 days[]
            = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

        if (month == 2 && IsLeapYear(year)) return 29;
        return days[month];
    }

    class Timestep
    {
      public:
        inline Timestep() = default;
        inline Timestep(u64 ns)
            : m_NanoSeconds(ns)
        {
        }

        inline      operator u64() const { return m_NanoSeconds; }

        inline u64  Seconds() const { return Milliseconds() / 1000; }
        inline u64  Milliseconds() const { return Microseconds() / 1000; }
        inline u64  Microseconds() const { return Nanoseconds() / 1000; }
        inline u64  Nanoseconds() const { return m_NanoSeconds; }

        inline void Set(u64 ns) { m_NanoSeconds = ns; }

        inline auto operator<=>(const Timestep& other) const = default;

      private:
        u64 m_NanoSeconds = 0;
    };

    struct DateTime
    {
        usize Year;
        u8    Month;
        u8    Day;
        u8    Hour;
        u8    Minute;
        u8    Second;

        constexpr DateTime() = default;
        constexpr DateTime(u64 epoch)
        {
            u64 daysSinceEpoch   = epoch / SECONDS_PER_DAY;
            u64 remainingSeconds = epoch % SECONDS_PER_DAY;

            Hour                 = remainingSeconds / SECONDS_PER_HOUR;
            remainingSeconds %= SECONDS_PER_HOUR;
            Minute = remainingSeconds / SECONDS_PER_MINUTE;
            Second = remainingSeconds % SECONDS_PER_MINUTE;

            Year   = 1970;
            for (;;)
            {
                u16 daysInYear = IsLeapYear(Year) ? 366 : 365;
                if (daysSinceEpoch < daysInYear) break;
                daysSinceEpoch -= daysInYear;
                ++Year;
            }

            Month = 1;
            while (daysSinceEpoch >= DaysInMonth(Month, Year))
            {
                daysSinceEpoch -= DaysInMonth(Month, Year);
                ++Month;
            }

            Day = daysSinceEpoch + 1;
        }

        constexpr usize DaysSinceEpoch() const
        {
            usize days = 0;
            for (u16 year = 1970; year < Year; year++)
                days += 365 + IsLeapYear(year);
            return days;
        }
        constexpr isize ToEpoch() const
        {
            usize days = DaysSinceEpoch();

            for (u8 month = 1; month < Month; month++)
                days += DaysInMonth(month, Year);

            days += Day - 1;
            return (days * 86400) + ((Hour - 1) * 3600) + (Minute * 60)
                 + Second;
        }
    };
}; // namespace Prism::Time

template <>
struct fmt::formatter<Prism::Time::DateTime> : fmt::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const Prism::Time::DateTime& date, FormatContext& ctx) const
    {
        return fmt::formatter<std::string>::format(
            fmt::format("{:02}:{:02}:{:04} {:02}:{:02}:{:02}", date.Day,
                        date.Month, date.Year, date.Hour, date.Minute,
                        date.Second),
            ctx);
    }
};

#if PRISM_TARGET_CRYPTIX == 1
using Prism::Time::DateTime;
using Prism::Time::DaysInMonth;
using Prism::Time::IsLeapYear;
#endif
