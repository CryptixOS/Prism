/*
 * Created by v1tr10l7 on 26.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Compiler.hpp>
#include <Prism/Config.hpp>
#include <Prism/Types.hpp>

namespace Prism
{
    class SourceLocation
    {
      public:
        constexpr SourceLocation() = default;
        SourceLocation(const SourceLocation& other) PM_NOEXCEPT
        {
            m_Line         = other.m_Line;
            m_Column       = other.m_Column;
            m_FileName     = other.m_FileName;
            m_FunctionName = other.m_FunctionName;
        }
        SourceLocation(SourceLocation&& other) PM_NOEXCEPT
        {
            m_Line         = other.m_Line;
            m_Column       = other.m_Column;
            m_FileName     = other.m_FileName;
            m_FunctionName = other.m_FunctionName;

            other.m_Line.reset();
            other.m_Column.reset();
            other.m_FileName.remove_prefix(other.m_FileName.size());
            other.m_FunctionName.remove_prefix(other.m_FunctionName.size());
        }

        static consteval SourceLocation
        Current(u32 line = PM_LINE, u32 column = PM_COLUMN,
                const char* fileName     = PM_FILENAME,
                const char* functionName = PM_FUNCTION_NAME) PM_NOEXCEPT
        {
            return SourceLocation(line, column, fileName, functionName);
        }

        constexpr u32 Line() const PM_NOEXCEPT { return m_Line.value_or(0); }
        constexpr u32 Column() const PM_NOEXCEPT
        {
            return m_Column.value_or(0);
        }
        constexpr std::string_view FileName() const PM_NOEXCEPT
        {
            return m_FileName;
        }
        constexpr std::string_view FunctionName() const PM_NOEXCEPT
        {
            return m_FunctionName.data();
        }

      private:
        std::optional<u32> m_Line         = 0;
        std::optional<u32> m_Column       = 0;
        std::string_view   m_FileName     = "";
        std::string_view   m_FunctionName = "";

        constexpr SourceLocation(u32 line, u32 column,
                                 std::string_view fileName,
                                 std::string_view functionName)
            : m_Line(line)
            , m_Column(column)
            , m_FileName(fileName)
            , m_FunctionName(functionName)
        {
        }
    };
}; // namespace Prism

template <>
struct fmt::formatter<Prism::SourceLocation> : fmt::formatter<std::string>
{
    template <typename FormatContext>
    auto format(const Prism::SourceLocation& src, FormatContext& ctx) const
    {
        return fmt::formatter<std::string>::format(
            fmt::format("{}[{}:{}]::{}", src.FileName(), src.Line(),
                        src.Column(), src.FunctionName()),
            ctx);
    }
};
