/*
 * Created by v1tr10l7 on 29.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
#include <Prism/Core/Config.hpp>

export module Prism.Core.NonCopyable;

export extern "C++"
{
    namespace Prism
    {
        template <typename T>
        class NonCopyable
        {
          public:
            NonCopyable(const NonCopyable&) = delete;
            T& operator=(const T&)          = delete;

          protected:
            NonCopyable()          = default;
            virtual ~NonCopyable() = default;
        };
    }; // namespace Prism

#ifdef PRISM_USE_NAMESPACE
    using Prism::NonCopyable;
#endif
}
