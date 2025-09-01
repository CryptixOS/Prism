/*
 * Created by v1tr10l7 on 02.09.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Types.hpp>

namespace Prism
{
    // FIXME(v1tr10l7): This currently has been copied from
    // `Prism/Containers/UnorderedMap.hpp`, eventually it will be removed from
    // there, however i don't want to do this at the moment, since it would
    // trigger recompilation of the whole project(CryptixOS)

    /**
     * @brief Structure holding a key-value pair.
     */
    template <typename K, typename V>
    struct KeyValuePair
    {
        using KeyType   = K;
        using ValueType = V;

        ///> The key
        KeyType   Key;
        ///> The value
        ValueType Value;

        /// @brief Default constructor.
        constexpr KeyValuePair() = default;
        /**
         * @brief Construct a key-value pair with lvalue reference.
         * @param key The key.
         * @param value The value.
         */
        constexpr KeyValuePair(const KeyType& key, const ValueType& value)
            : Key(key)
            , Value(value)
        {
        }
        /**
         * @brief Construct a key-value pair with rvalue reference for
         * value.
         * @param key The key.
         * @param value The value to move.
         */
        constexpr KeyValuePair(const KeyType& key, ValueType&& value)
            : Key(key)
            , Value(Move(value))
        {
        }
    };

    template <typename T1, typename T2>
    using synth3way_t = std::__detail::__synth3way_t<T1, T2>;
    template <typename T1, typename T2, typename U1, typename U2>
    constexpr std::common_comparison_category_t<synth3way_t<T1, U1>,
                                                synth3way_t<T2, U2>>
    operator<=>(const KeyValuePair<T1, T2>& lhs,
                const KeyValuePair<U1, U2>& rhs);
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX != 0
using Prism::KeyValuePair;
#endif
