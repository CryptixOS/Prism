/*
 * Created by v1tr10l7 on 06.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <string_view>

namespace Prism
{
    template <typename Char, typename... Args>
    class BasicFormatString
    {
      public:
        template <typename S,
                  std::enable_if<std::is_convertible<
                      const S&, std::basic_string_view<Char>>::value>>
        consteval BasicFormatString(const S& str)
            : m_String(str)
        {
            static_assert(
                detail::count<(std::is_base_of<detail::view,
                                               remove_reference_t<Args>>::value
                               && std::is_reference<Args>::value)...>()
                    == 0,
                "passing views as lvalues is disallowed");
            if constexpr (detail::count_named_args<Args...>()
                          == detail::count_statically_named_args<Args...>())
            {
                using checker
                    = detail::format_string_checker<Char,
                                                    remove_cvref_t<Args>...>;
                detail::parse_format_string<true>(str_, checker(s));
            }

        )
        }

      private:
        std::basic_string_view<Char> m_String;
    };
}; // namespace Prism
