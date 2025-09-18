/*
 * Created by v1tr10l7 on 26.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

namespace Prism
{
    template <typename T>
    class Singleton
    {
      public:
        // Gets instance of an object
        static T* Instance()
        {
            static T instance;

            return &instance;
        }

      protected:
        Singleton()  = default;
        ~Singleton() = default;

      private:
        Singleton(Singleton const&)            = default;
        Singleton& operator=(Singleton const&) = default;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::Singleton;
#endif
