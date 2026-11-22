/*
 * Created by v1tr10l7 on 04.03.2025.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Config.hpp>
#include <Prism/Core/Types.hpp>

export module Prism.Utility.OptionParser;

export import Prism.String.StringView;

export extern "C++"
{
    namespace Prism
    {
        /// A long option, laid out like getopt_long's `struct option`. The
        /// table passed to OptionParser ends with an entry whose Name is
        /// nullptr.
        struct LongOption
        {
            static constexpr i32 eNoArgument       = 0;
            static constexpr i32 eRequiredArgument = 1;
            static constexpr i32 eOptionalArgument = 2;

            const char*          Name;
            /// eNoArgument, eRequiredArgument or eOptionalArgument.
            i32                  HasArgument;
            /// If set, a match stores Value here and Next() returns 0.
            i32*                 Flag;
            i32                  Value;
        };

        /**
         * @brief Command-line option parser with the semantics of GNU getopt,
         * getopt_long and getopt_long_only, without global state, allocation
         * or libc. A libc can implement getopt() on top of one instance.
         *
         * Short options use getopt's syntax: "ab:c::" is -a, -b with a
         * required argument and -c with an optional one. A leading '+' stops
         * at the first non-option (POSIX order), a leading '-' returns
         * non-options as option 1, and a following ':' returns ':' instead
         * of '?' for a missing argument (and disables error reports). "W;"
         * makes -W foo mean --foo.
         *
         * By default non-options are moved behind the options (argv is
         * permuted), so after Next() returns -1, Index is the first
         * non-option. Setting Index to 0 restarts parsing.
         */
        class OptionParser
        {
          public:
            /// Called with a message such as "invalid option -- 'x'", when
            /// PrintErrors is set and the short option string doesn't start
            /// with ':'.
            using ErrorHandler = void (*)(const char* program,
                                          StringView  message);

            /// Index of the next argv element (optind).
            i32          Index        = 1;
            /// Argument of the option just returned, or nullptr (optarg).
            char*        Argument     = nullptr;
            /// The offending option character after an error (optopt).
            i32          Option       = '?';
            /// Whether errors are reported through OnError (opterr).
            i32          PrintErrors  = 1;
            /// Stop at the first non-option, as with a leading '+'
            /// (the POSIXLY_CORRECT environment variable of glibc).
            bool         PosixlyCorrect = false;
            ErrorHandler OnError        = nullptr;

            /// getopt(): the next short option, '?' or ':' on errors, or -1
            /// when there are no more options.
            i32 Next(i32 argc, char** argv, const char* shortOptions)
            {
                return Next(argc, argv, shortOptions, nullptr, nullptr, false);
            }
            /// getopt_long() (longOnly = false) and getopt_long_only()
            /// (longOnly = true, "-name" may be a long option too). For a
            /// long option, *longIndex is set to its index in longOptions.
            i32 Next(i32 argc, char** argv, const char* shortOptions,
                     const LongOption* longOptions, i32* longIndex,
                     bool longOnly = false);

          private:
            enum class Ordering
            {
                ePermute,
                eRequireOrder,
                eReturnInOrder,
            };

            char*    m_NextChar       = nullptr;
            i32      m_FirstNonOption = 1;
            i32      m_LastNonOption  = 1;
            Ordering m_Ordering       = Ordering::ePermute;
            bool     m_Initialized    = false;

            void     Exchange(char** argv);
            i32 ProcessLongOption(i32 argc, char** argv, const char* shortOptions,
                                  const LongOption* longOptions, i32* longIndex,
                                  bool longOnly, bool printErrors,
                                  const char* prefix);
            /// Reports the message made of the given pieces.
            void Report(char** argv, bool printErrors, StringView a,
                        StringView b = {}, StringView c = {},
                        StringView d = {});
        };
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::LongOption;
    using Prism::OptionParser;
#endif
}
