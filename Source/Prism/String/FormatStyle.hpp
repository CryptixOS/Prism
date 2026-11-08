/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/String/Formatter.hpp>

/**
 * Text styles (fmt/color.h): ANSI foreground/background colors and
 * emphasis.
 *
 *   Format(Fg(TextColor::eRed) | TextEmphasis::eBold, "error: {}", msg)
 *   Format("{} {}", Styled(42, Bg(TerminalColor::eBlue)), "plain")
 */
namespace Prism
{
    /// Named 24-bit colors (the CSS/X11 set, same values as fmt::color).
    enum class TextColor : u32
    {
        eAliceBlue            = 0xF0F8FF,
        eAntiqueWhite         = 0xFAEBD7,
        eAqua                 = 0x00FFFF,
        eAquamarine           = 0x7FFFD4,
        eAzure                = 0xF0FFFF,
        eBeige                = 0xF5F5DC,
        eBisque               = 0xFFE4C4,
        eBlack                = 0x000000,
        eBlanchedAlmond       = 0xFFEBCD,
        eBlue                 = 0x0000FF,
        eBlueViolet           = 0x8A2BE2,
        eBrown                = 0xA52A2A,
        eBurlyWood            = 0xDEB887,
        eCadetBlue            = 0x5F9EA0,
        eChartreuse           = 0x7FFF00,
        eChocolate            = 0xD2691E,
        eCoral                = 0xFF7F50,
        eCornflowerBlue       = 0x6495ED,
        eCornsilk             = 0xFFF8DC,
        eCrimson              = 0xDC143C,
        eCyan                 = 0x00FFFF,
        eDarkBlue             = 0x00008B,
        eDarkCyan             = 0x008B8B,
        eDarkGoldenRod        = 0xB8860B,
        eDarkGray             = 0xA9A9A9,
        eDarkGreen            = 0x006400,
        eDarkKhaki            = 0xBDB76B,
        eDarkMagenta          = 0x8B008B,
        eDarkOliveGreen       = 0x556B2F,
        eDarkOrange           = 0xFF8C00,
        eDarkOrchid           = 0x9932CC,
        eDarkRed              = 0x8B0000,
        eDarkSalmon           = 0xE9967A,
        eDarkSeaGreen         = 0x8FBC8F,
        eDarkSlateBlue        = 0x483D8B,
        eDarkSlateGray        = 0x2F4F4F,
        eDarkTurquoise        = 0x00CED1,
        eDarkViolet           = 0x9400D3,
        eDeepPink             = 0xFF1493,
        eDeepSkyBlue          = 0x00BFFF,
        eDimGray              = 0x696969,
        eDodgerBlue           = 0x1E90FF,
        eFireBrick            = 0xB22222,
        eFloralWhite          = 0xFFFAF0,
        eForestGreen          = 0x228B22,
        eFuchsia              = 0xFF00FF,
        eGainsboro            = 0xDCDCDC,
        eGhostWhite           = 0xF8F8FF,
        eGold                 = 0xFFD700,
        eGoldenRod            = 0xDAA520,
        eGray                 = 0x808080,
        eGreen                = 0x008000,
        eGreenYellow          = 0xADFF2F,
        eHoneyDew             = 0xF0FFF0,
        eHotPink              = 0xFF69B4,
        eIndianRed            = 0xCD5C5C,
        eIndigo               = 0x4B0082,
        eIvory                = 0xFFFFF0,
        eKhaki                = 0xF0E68C,
        eLavender             = 0xE6E6FA,
        eLavenderBlush        = 0xFFF0F5,
        eLawnGreen            = 0x7CFC00,
        eLemonChiffon         = 0xFFFACD,
        eLightBlue            = 0xADD8E6,
        eLightCoral           = 0xF08080,
        eLightCyan            = 0xE0FFFF,
        eLightGoldenRodYellow = 0xFAFAD2,
        eLightGray            = 0xD3D3D3,
        eLightGreen           = 0x90EE90,
        eLightPink            = 0xFFB6C1,
        eLightSalmon          = 0xFFA07A,
        eLightSeaGreen        = 0x20B2AA,
        eLightSkyBlue         = 0x87CEFA,
        eLightSlateGray       = 0x778899,
        eLightSteelBlue       = 0xB0C4DE,
        eLightYellow          = 0xFFFFE0,
        eLime                 = 0x00FF00,
        eLimeGreen            = 0x32CD32,
        eLinen                = 0xFAF0E6,
        eMagenta              = 0xFF00FF,
        eMaroon               = 0x800000,
        eMediumAquamarine     = 0x66CDAA,
        eMediumBlue           = 0x0000CD,
        eMediumOrchid         = 0xBA55D3,
        eMediumPurple         = 0x9370DB,
        eMediumSeaGreen       = 0x3CB371,
        eMediumSlateBlue      = 0x7B68EE,
        eMediumSpringGreen    = 0x00FA9A,
        eMediumTurquoise      = 0x48D1CC,
        eMediumVioletRed      = 0xC71585,
        eMidnightBlue         = 0x191970,
        eMintCream            = 0xF5FFFA,
        eMistyRose            = 0xFFE4E1,
        eMoccasin             = 0xFFE4B5,
        eNavajoWhite          = 0xFFDEAD,
        eNavy                 = 0x000080,
        eOldLace              = 0xFDF5E6,
        eOlive                = 0x808000,
        eOliveDrab            = 0x6B8E23,
        eOrange               = 0xFFA500,
        eOrangeRed            = 0xFF4500,
        eOrchid               = 0xDA70D6,
        ePaleGoldenRod        = 0xEEE8AA,
        ePaleGreen            = 0x98FB98,
        ePaleTurquoise        = 0xAFEEEE,
        ePaleVioletRed        = 0xDB7093,
        ePapayaWhip           = 0xFFEFD5,
        ePeachPuff            = 0xFFDAB9,
        ePeru                 = 0xCD853F,
        ePink                 = 0xFFC0CB,
        ePlum                 = 0xDDA0DD,
        ePowderBlue           = 0xB0E0E6,
        ePurple               = 0x800080,
        eRebeccaPurple        = 0x663399,
        eRed                  = 0xFF0000,
        eRosyBrown            = 0xBC8F8F,
        eRoyalBlue            = 0x4169E1,
        eSaddleBrown          = 0x8B4513,
        eSalmon               = 0xFA8072,
        eSandyBrown           = 0xF4A460,
        eSeaGreen             = 0x2E8B57,
        eSeaShell             = 0xFFF5EE,
        eSienna               = 0xA0522D,
        eSilver               = 0xC0C0C0,
        eSkyBlue              = 0x87CEEB,
        eSlateBlue            = 0x6A5ACD,
        eSlateGray            = 0x708090,
        eSnow                 = 0xFFFAFA,
        eSpringGreen          = 0x00FF7F,
        eSteelBlue            = 0x4682B4,
        eTan                  = 0xD2B48C,
        eTeal                 = 0x008080,
        eThistle              = 0xD8BFD8,
        eTomato               = 0xFF6347,
        eTurquoise            = 0x40E0D0,
        eViolet               = 0xEE82EE,
        eWheat                = 0xF5DEB3,
        eWhite                = 0xFFFFFF,
        eWhiteSmoke           = 0xF5F5F5,
        eYellow               = 0xFFFF00,
        eYellowGreen          = 0x9ACD32,
    };

    /// The 16 standard terminal colors (SGR foreground codes).
    enum class TerminalColor : u8
    {
        eBlack = 30,
        eRed,
        eGreen,
        eYellow,
        eBlue,
        eMagenta,
        eCyan,
        eWhite,
        eBrightBlack = 90,
        eBrightRed,
        eBrightGreen,
        eBrightYellow,
        eBrightBlue,
        eBrightMagenta,
        eBrightCyan,
        eBrightWhite,
    };

    enum class TextEmphasis : u8
    {
        eBold          = 1,
        eFaint         = 1 << 1,
        eItalic        = 1 << 2,
        eUnderline     = 1 << 3,
        eBlink         = 1 << 4,
        eReverse       = 1 << 5,
        eConceal       = 1 << 6,
        eStrikethrough = 1 << 7,
    };

    struct RgbColor
    {
        u8 R                 = 0;
        u8 G                 = 0;
        u8 B                 = 0;

        constexpr RgbColor() = default;
        constexpr RgbColor(u8 r, u8 g, u8 b)
            : R(r)
            , G(g)
            , B(b)
        {
        }
        constexpr RgbColor(u32 hex)
            : R((hex >> 16) & 0xff)
            , G((hex >> 8) & 0xff)
            , B(hex & 0xff)
        {
        }
        constexpr RgbColor(TextColor color)
            : RgbColor(static_cast<u32>(color))
        {
        }
    };

    /// Either a 24-bit color or one of the terminal colors.
    struct StyleColor
    {
        bool IsRgb = true;
        union
        {
            struct RgbColor Rgb;
            TerminalColor   Terminal;
        };

        constexpr StyleColor()
            : Rgb()
        {
        }
        constexpr StyleColor(struct RgbColor rgb)
            : Rgb(rgb)
        {
        }
        constexpr StyleColor(TextColor color)
            : Rgb(color)
        {
        }
        constexpr StyleColor(u32 hex)
            : Rgb(hex)
        {
        }
        constexpr StyleColor(TerminalColor color)
            : IsRgb(false)
            , Terminal(color)
        {
        }
    };

    class TextStyle
    {
      public:
        constexpr TextStyle() = default;
        constexpr TextStyle(TextEmphasis emphasis)
            : m_Emphasis(static_cast<u8>(emphasis))
        {
        }

        constexpr bool HasForeground() const { return m_HasForeground; }
        constexpr bool HasBackground() const { return m_HasBackground; }
        constexpr bool HasEmphasis() const { return m_Emphasis != 0; }
        constexpr bool Empty() const
        { return !HasForeground() && !HasBackground() && !HasEmphasis(); }

        constexpr StyleColor Foreground() const { return m_Foreground; }
        constexpr StyleColor Background() const { return m_Background; }
        constexpr u8         EmphasisMask() const { return m_Emphasis; }

        /// Combines two styles; colors set in `rhs` win.
        constexpr TextStyle& operator|=(const TextStyle& rhs)
        {
            if (rhs.m_HasForeground)
            {
                m_Foreground    = rhs.m_Foreground;
                m_HasForeground = true;
            }
            if (rhs.m_HasBackground)
            {
                m_Background    = rhs.m_Background;
                m_HasBackground = true;
            }
            m_Emphasis |= rhs.m_Emphasis;
            return *this;
        }
        friend constexpr TextStyle operator|(TextStyle        lhs,
                                             const TextStyle& rhs)
        { return lhs |= rhs; }

        static constexpr TextStyle MakeForeground(StyleColor color)
        {
            TextStyle style;
            style.m_Foreground    = color;
            style.m_HasForeground = true;
            return style;
        }
        static constexpr TextStyle MakeBackground(StyleColor color)
        {
            TextStyle style;
            style.m_Background    = color;
            style.m_HasBackground = true;
            return style;
        }

      private:
        StyleColor m_Foreground;
        StyleColor m_Background;
        bool       m_HasForeground = false;
        bool       m_HasBackground = false;
        u8         m_Emphasis      = 0;
    };

    constexpr TextStyle operator|(TextEmphasis lhs, TextEmphasis rhs)
    { return TextStyle(lhs) | TextStyle(rhs); }

    /// Foreground color style (fmt::fg).
    constexpr TextStyle Fg(StyleColor color)
    { return TextStyle::MakeForeground(color); }
    /// Background color style (fmt::bg).
    constexpr TextStyle Bg(StyleColor color)
    { return TextStyle::MakeBackground(color); }

    namespace Details
    {
        inline void WriteDecimal(FormatBuffer& out, u32 value)
        {
            char  digits[12];
            char* end = digits + sizeof(digits);
            usize n   = FormatDigits(end, value, 10, false, false);
            out.Append(end - n, n);
        }
        /// fmt 10.2 always writes RGB components as three digits.
        inline void WriteColorComponent(FormatBuffer& out, u8 value)
        {
            out.PushBack(static_cast<char>('0' + value / 100));
            out.PushBack(static_cast<char>('0' + value / 10 % 10));
            out.PushBack(static_cast<char>('0' + value % 10));
        }
        inline void WriteStyleColor(FormatBuffer& out, StyleColor color,
                                    bool background)
        {
            out.Append("\x1b[", 2);
            if (!color.IsRgb)
                WriteDecimal(out, static_cast<u32>(color.Terminal)
                                      + (background ? 10 : 0));
            else
            {
                out.Append(background ? "48;2;" : "38;2;", 5);
                WriteColorComponent(out, color.Rgb.R);
                out.PushBack(';');
                WriteColorComponent(out, color.Rgb.G);
                out.PushBack(';');
                WriteColorComponent(out, color.Rgb.B);
            }
            out.PushBack('m');
        }

        /// Emits the escape sequences for `style`; returns whether anything
        /// was written (and thus needs a reset afterwards).
        inline bool WriteStyleBegin(FormatBuffer& out, const TextStyle& style)
        {
            if (style.HasEmphasis())
            {
                constexpr u8 codes[] = {1, 2, 3, 4, 5, 7, 8, 9};
                for (usize i = 0; i < 8; ++i)
                {
                    if (!(style.EmphasisMask() & (1u << i))) continue;
                    out.Append("\x1b[", 2);
                    WriteDecimal(out, codes[i]);
                    out.PushBack('m');
                }
            }
            if (style.HasForeground())
                WriteStyleColor(out, style.Foreground(), false);
            if (style.HasBackground())
                WriteStyleColor(out, style.Background(), true);
            return !style.Empty();
        }
        inline void WriteStyleReset(FormatBuffer& out)
        { out.Append("\x1b[0m", 4); }
    } // namespace Details

    inline void VFormatTo(FormatBuffer& out, const TextStyle& style,
                          StringView format, FormatArgs args)
    {
        bool styled = Details::WriteStyleBegin(out, style);
        VFormatTo(out, format, args);
        if (styled) Details::WriteStyleReset(out);
    }

    template <typename Sink>
    void VFormatToSink(Sink&& sink, const TextStyle& style, StringView format,
                       FormatArgs args)
    {
        using Ref = Details::SinkRef<RemoveReferenceType<Sink>>;
        SinkFormatBuffer<Ref> buffer(Ref{&sink});
        VFormatTo(buffer, style, format, args);
        buffer.Flush();
    }

    PM_NODISCARD inline String VFormat(const TextStyle& style,
                                       StringView format, FormatArgs args)
    {
        String result;
        VFormatToSink([&result](const char* data, usize size)
                      { result += StringView(data, size); }, style, format,
                      args);
        return result;
    }
    template <typename... Args>
    PM_NODISCARD String Format(const TextStyle&      style,
                               FormatString<Args...> format, Args&&... args)
    { return VFormat(style, format.View(), MakeFormatArgs(args...)); }

    template <typename OutputIt, typename... Args>
        requires FormatOutputIterator<OutputIt>
    OutputIt FormatTo(OutputIt out, const TextStyle& style,
                      FormatString<Args...> format, Args&&... args)
    {
        auto formatArgs = MakeFormatArgs(args...);
        if constexpr (IsSameV<OutputIt, FormatAppender>)
        {
            VFormatTo(out.Buffer(), style, format.View(), formatArgs);
            return out;
        }
        else
        {
            VFormatToSink(
                [&out](const char* data, usize size)
                {
                    for (usize i = 0; i < size; ++i) *out++ = data[i];
                },
                style, format.View(), formatArgs);
            return out;
        }
    }
    template <typename Container, typename... Args>
        requires FormatAppendable<Container>
    Container& FormatTo(Container& out, const TextStyle& style,
                        FormatString<Args...> format, Args&&... args)
    {
        String styled = Format(style, format, Forward<Args>(args)...);
        return VFormatTo(out, "{}", MakeFormatArgs(styled));
    }

    template <typename... Args>
    void Print(const TextStyle& style, FormatString<Args...> format,
               Args&&... args)
    {
        VFormatToSink([](const char* data, usize size)
                      { Log::Print(StringView(data, size)); }, style,
                      format.View(), MakeFormatArgs(args...));
    }

    /// A value printed with a style (fmt::styled).
    template <typename T>
    struct StyledArg
    {
        const T&  Value;
        TextStyle Style;
    };
    template <typename T>
    constexpr StyledArg<T> Styled(const T& value, TextStyle style)
    { return {value, style}; }
    template <typename T>
    struct Formatter<StyledArg<T>, char>
    {
        Details::FormatterFor<Details::FormatKeyType<T>> Inner{};

        constexpr const char* Parse(FormatParseContext& ctx)
        { return Details::ParseWith(Inner, ctx); }
        FormatAppender Format(const StyledArg<T>& arg, FormatContext& ctx) const
        {
            bool styled = Details::WriteStyleBegin(ctx.Buffer(), arg.Style);
            auto inner  = Inner;
            inner.Format(arg.Value, ctx);
            if (styled) Details::WriteStyleReset(ctx.Buffer());
            return ctx.Out();
        }
    };
}; // namespace Prism
