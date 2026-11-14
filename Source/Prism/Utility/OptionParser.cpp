/*
 * Created by v1tr10l7 on 04.03.2025.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Core/Types.hpp>
import Prism.Utility.OptionParser;

namespace Prism
{
    namespace
    {
        usize Length(const char* s)
        {
            usize n = 0;
            while (s[n]) ++n;
            return n;
        }
        /// strchr() for the short option string: nullptr for '\0'.
        const char* FindShort(const char* options, char c)
        {
            if (c == '\0') return nullptr;
            for (; *options; ++options)
                if (*options == c) return options;
            return nullptr;
        }
        bool StartsWith(const char* name, const char* prefix, usize length)
        {
            for (usize i = 0; i < length; ++i)
                if (name[i] != prefix[i]) return false;
            return true;
        }
        bool IsNonOption(const char* arg)
        {
            return arg[0] != '-' || arg[1] == '\0';
        }
        void Reverse(char** first, char** last)
        {
            while (first < last && first < --last)
            {
                char* tmp = *first;
                *first++  = *last;
                *last     = tmp;
            }
        }
    } // namespace

    void OptionParser::Report(char** argv, bool printErrors, StringView a,
                              StringView b, StringView c, StringView d)
    {
        if (!printErrors || !OnError) return;

        char       buffer[256];
        usize      size     = 0;
        StringView pieces[] = {a, b, c, d};
        for (StringView piece : pieces)
            for (usize i = 0; i < piece.Size() && size + 1 < sizeof(buffer); ++i)
                buffer[size++] = piece[i];
        buffer[size] = '\0';
        OnError(argv[0], StringView(buffer, size));
    }

    // Moves the block of non-options [m_FirstNonOption, m_LastNonOption)
    // behind the block of options [m_LastNonOption, Index), keeping the order
    // within each block.
    void OptionParser::Exchange(char** argv)
    {
        Reverse(argv + m_FirstNonOption, argv + m_LastNonOption);
        Reverse(argv + m_LastNonOption, argv + Index);
        Reverse(argv + m_FirstNonOption, argv + Index);

        m_FirstNonOption += Index - m_LastNonOption;
        m_LastNonOption = Index;
    }

    i32 OptionParser::ProcessLongOption(i32 argc, char** argv,
                                        const char*       shortOptions,
                                        const LongOption* longOptions,
                                        i32* longIndex, bool longOnly,
                                        bool printErrors, const char* prefix)
    {
        char* nameEnd = m_NextChar;
        while (*nameEnd && *nameEnd != '=') ++nameEnd;
        usize             nameLength = nameEnd - m_NextChar;
        StringView        name(m_NextChar, nameLength);

        const LongOption* found      = nullptr;
        i32               foundIndex = -1;

        // An exact match wins over abbreviations.
        for (i32 i = 0; longOptions[i].Name; ++i)
        {
            const LongOption& option = longOptions[i];
            if (Length(option.Name) == nameLength
                && StartsWith(option.Name, m_NextChar, nameLength))
            {
                found      = &option;
                foundIndex = i;
                break;
            }
        }
        if (!found)
        {
            bool ambiguous = false;
            for (i32 i = 0; longOptions[i].Name; ++i)
            {
                const LongOption& option = longOptions[i];
                if (!StartsWith(option.Name, m_NextChar, nameLength)
                    || Length(option.Name) < nameLength)
                    continue;
                if (!found)
                {
                    found      = &option;
                    foundIndex = i;
                }
                // Two abbreviations of options that behave the same (like
                // --colors and --colours) are not ambiguous.
                else if (longOnly || found->HasArgument != option.HasArgument
                         || found->Flag != option.Flag
                         || found->Value != option.Value)
                    ambiguous = true;
            }
            if (ambiguous)
            {
                Report(argv, printErrors, "option '", prefix, name,
                       "' is ambiguous");
                m_NextChar += Length(m_NextChar);
                ++Index;
                Option = 0;
                return '?';
            }
        }

        if (!found)
        {
            // getopt_long_only falls back to short options for "-xyz" when
            // x is one.
            if (!longOnly || argv[Index][1] == '-'
                || !FindShort(shortOptions, *m_NextChar))
            {
                Report(argv, printErrors, "unrecognized option '", prefix,
                       m_NextChar, "'");
                m_NextChar = nullptr;
                ++Index;
                Option = 0;
                return '?';
            }
            return -1;
        }

        ++Index;
        m_NextChar = nullptr;
        if (*nameEnd)
        {
            if (found->HasArgument == LongOption::eNoArgument)
            {
                Report(argv, printErrors, "option '", prefix, found->Name,
                       "' doesn't allow an argument");
                Option = found->Value;
                return '?';
            }
            Argument = nameEnd + 1;
        }
        else if (found->HasArgument == LongOption::eRequiredArgument)
        {
            if (Index >= argc)
            {
                Report(argv, printErrors, "option '", prefix, found->Name,
                       "' requires an argument");
                Option = found->Value;
                return shortOptions[0] == ':' ? ':' : '?';
            }
            Argument = argv[Index++];
        }

        if (longIndex) *longIndex = foundIndex;
        if (found->Flag)
        {
            *found->Flag = found->Value;
            return 0;
        }
        return found->Value;
    }

    i32 OptionParser::Next(i32 argc, char** argv, const char* shortOptions,
                           const LongOption* longOptions, i32* longIndex,
                           bool longOnly)
    {
        if (argc < 1) return -1;
        Argument = nullptr;

        if (Index == 0 || !m_Initialized)
        {
            if (Index == 0) Index = 1;
            m_FirstNonOption = m_LastNonOption = Index;
            m_NextChar                         = nullptr;
            if (shortOptions[0] == '-') m_Ordering = Ordering::eReturnInOrder;
            else if (shortOptions[0] == '+' || PosixlyCorrect)
                m_Ordering = Ordering::eRequireOrder;
            else m_Ordering = Ordering::ePermute;
            m_Initialized = true;
        }
        if (shortOptions[0] == '-' || shortOptions[0] == '+') ++shortOptions;
        bool printErrors = PrintErrors && shortOptions[0] != ':';

        if (!m_NextChar || *m_NextChar == '\0')
        {
            // Index may have been moved back by the caller.
            if (m_LastNonOption > Index) m_LastNonOption = Index;
            if (m_FirstNonOption > Index) m_FirstNonOption = Index;

            if (m_Ordering == Ordering::ePermute)
            {
                if (m_FirstNonOption != m_LastNonOption
                    && m_LastNonOption != Index)
                    Exchange(argv);
                else if (m_LastNonOption != Index) m_FirstNonOption = Index;

                while (Index < argc && IsNonOption(argv[Index])) ++Index;
                m_LastNonOption = Index;
            }

            // "--" ends the options; what follows are non-options.
            if (Index != argc && argv[Index][0] == '-' && argv[Index][1] == '-'
                && argv[Index][2] == '\0')
            {
                ++Index;
                if (m_FirstNonOption != m_LastNonOption
                    && m_LastNonOption != Index)
                    Exchange(argv);
                else if (m_FirstNonOption == m_LastNonOption)
                    m_FirstNonOption = Index;
                m_LastNonOption = argc;
                Index           = argc;
            }

            if (Index == argc)
            {
                // Point at the non-options that were moved to the end.
                if (m_FirstNonOption != m_LastNonOption)
                    Index = m_FirstNonOption;
                return -1;
            }

            if (IsNonOption(argv[Index]))
            {
                if (m_Ordering == Ordering::eRequireOrder) return -1;
                Argument = argv[Index++];
                return 1;
            }

            if (longOptions)
            {
                if (argv[Index][1] == '-')
                {
                    m_NextChar = argv[Index] + 2;
                    return ProcessLongOption(argc, argv, shortOptions,
                                             longOptions, longIndex, longOnly,
                                             printErrors, "--");
                }
                if (longOnly
                    && (argv[Index][2] != '\0'
                        || !FindShort(shortOptions, argv[Index][1])))
                {
                    m_NextChar = argv[Index] + 1;
                    i32 code   = ProcessLongOption(argc, argv, shortOptions,
                                                   longOptions, longIndex,
                                                   longOnly, printErrors, "-");
                    if (code != -1) return code;
                }
            }
            m_NextChar = argv[Index] + 1;
        }

        // A short option, possibly one of several in one element ("-abc").
        char        c       = *m_NextChar++;
        const char* spec    = FindShort(shortOptions, c);
        char        name[2] = {c, '\0'};

        if (*m_NextChar == '\0') ++Index;

        if (!spec || c == ':' || c == ';')
        {
            Report(argv, printErrors, "invalid option -- '", name, "'");
            Option = c;
            return '?';
        }

        // "W;": -W foo is --foo.
        if (spec[0] == 'W' && spec[1] == ';' && longOptions)
        {
            // The name is the rest of this element ("-Wfoo") or the next one.
            if (*m_NextChar == '\0' && Index == argc)
            {
                Report(argv, printErrors, "option requires an argument -- '",
                       name, "'");
                Option = c;
                return shortOptions[0] == ':' ? ':' : '?';
            }
            if (*m_NextChar == '\0') m_NextChar = argv[Index];
            return ProcessLongOption(argc, argv, shortOptions, longOptions,
                                     longIndex, false, printErrors, "-W ");
        }

        if (spec[1] == ':')
        {
            if (spec[2] == ':')
            {
                // Optional: only an argument in the same element counts.
                if (*m_NextChar != '\0')
                {
                    Argument = m_NextChar;
                    ++Index;
                }
            }
            else if (*m_NextChar != '\0')
            {
                Argument = m_NextChar;
                ++Index;
            }
            else if (Index == argc)
            {
                Report(argv, printErrors, "option requires an argument -- '",
                       name, "'");
                Option = c;
                c      = shortOptions[0] == ':' ? ':' : '?';
            }
            else Argument = argv[Index++];
            m_NextChar = nullptr;
        }
        return c;
    }
}; // namespace Prism
