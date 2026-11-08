/*
 * Created by v1tr10l7 on 07.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/TypeTraits.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/String/StringView.hpp>

namespace Prism::Test
{
    /**
     * Crash tests: run a function in a forked child process and check how
     * the child ended. See EXPECT_CRASH / EXPECT_CRASH_WITH_SIGNAL /
     * EXPECT_NO_CRASH.
     */
    class Crash
    {
      public:
        /// What the crash function reports when it returns instead of
        /// crashing.
        enum class Failure
        {
            eDidNotCrash,
            eUnexpectedError,
        };

        /// Signal value meaning "any signal is fine".
        static constexpr int AnySignal = -1;
        /// Default expected signal (SIGABRT, what failing assertions raise).
        static constexpr int DefaultSignal = 6;

        template <typename Fn>
        static bool Run(StringView description, Fn&& function,
                        int expectedSignal = DefaultSignal)
        {
            return RunImpl(description, &Thunk<RemoveReferenceType<Fn>>,
                           const_cast<void*>(static_cast<const void*>(&function)),
                           expectedSignal);
        }

      private:
        using ThunkFn = Failure (*)(void*);

        template <typename Fn>
        static Failure Thunk(void* context)
        {
            auto& function = *static_cast<Fn*>(context);
            if constexpr (IsVoidV<decltype(function())>)
            {
                function();
                return Failure::eDidNotCrash;
            }
            else return function();
        }

        static bool RunImpl(StringView description, ThunkFn thunk,
                            void* context, int expectedSignal);
    };
} // namespace Prism::Test
