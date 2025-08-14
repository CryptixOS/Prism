/*
 * Created by v1tr10l7 on 26.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Config.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/String/StringView.hpp>
#include <Prism/Utility/Optional.hpp>

namespace Prism
{
    class SourceLocation
    {
      public:
        constexpr SourceLocation() = default;
        constexpr SourceLocation(const SourceLocation& other) PM_NOEXCEPT
        {
            m_Line         = other.m_Line;
            m_Column       = other.m_Column;
            m_FileName     = other.m_FileName;
            m_FunctionName = other.m_FunctionName;
        }
        constexpr SourceLocation(SourceLocation&& other) PM_NOEXCEPT
        {
            m_Line         = other.m_Line;
            m_Column       = other.m_Column;
            m_FileName     = other.m_FileName;
            m_FunctionName = other.m_FunctionName;

            other.m_Line.Reset();
            other.m_Column.Reset();
            other.m_FileName.RemovePrefix(other.m_FileName.Size());
            other.m_FunctionName.RemovePrefix(other.m_FunctionName.Size());
        }

        static constexpr SourceLocation
        Current(i32 line = PM_LINE, i32 column = PM_COLUMN,
                const char* fileName     = PM_FILENAME,
                const char* functionName = PM_FUNCTION_NAME) PM_NOEXCEPT
        {
            return SourceLocation(line, column, fileName, functionName);
        }

        constexpr u32 Line() const PM_NOEXCEPT { return m_Line.ValueOr(0); }
        constexpr u32 Column() const PM_NOEXCEPT { return m_Column.ValueOr(0); }
        constexpr StringView FileName() const PM_NOEXCEPT { return m_FileName; }
        constexpr StringView FunctionName() const PM_NOEXCEPT
        {
            return m_FunctionName;
        }

      private:
        Optional<u32> m_Line         = 0;
        Optional<u32> m_Column       = 0;
        StringView    m_FileName     = "";
        StringView    m_FunctionName = "";

        constexpr SourceLocation(u32 line, u32 column, StringView fileName,
                                 StringView functionName)
            : m_Line(line)
            , m_Column(column)
            , m_FileName(fileName)
            , m_FunctionName(functionName)
        {
        }
    };
}; // namespace Prism

#if PRISM_DISABLE_FMT == 0
template <>
struct fmt::formatter<Prism::SourceLocation> : fmt::formatter<fmt::string_view>
{
    template <typename FormatContext>
    auto format(const Prism::SourceLocation& src, FormatContext& ctx) const
    {
        fmt::string_view filename
            = {src.FileName().Raw(), src.FileName().Size()};
        fmt::string_view functionName
            = {src.FunctionName().Raw(), src.FunctionName().Size()};

        return fmt::formatter<fmt::string_view>::format(
            fmt::format("{}[{}:{}]::{}", filename, src.Line(), src.Column(),
                        functionName),
            ctx);
    }
};
#endif
