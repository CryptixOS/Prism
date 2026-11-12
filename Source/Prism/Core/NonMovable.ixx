/*
 * Created by v1tr10l7 on 25.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
module;

#include <Prism/Core/Types.hpp>
#include <Prism/Core/Config.hpp>

export module Prism.Core.NonMovable;

export extern "C++"
{
    namespace Prism
    {
        template <typename T>
        class NonMovable
        {
          protected:
            NonMovable()          = default;
            virtual ~NonMovable() = default;

          private:
            NonMovable(NonMovable&&)            = delete;
            NonMovable& operator=(NonMovable&&) = delete;
        };
    }; // namespace Prism

#if PRISM_USE_NAMESPACE != 0
    using Prism::NonMovable;
#endif
}
