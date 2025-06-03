/*
 * Created by v1tr10l7 on 17.11.2024.
 * Copyright (c) 2024-2024, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Vector.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/Memory/Pointer.hpp>

#include <compare>

namespace Prism
{
    template <typename C>
    struct CharTraits;
    template <typename C, typename Traits>
    class BasicString;
    using String = BasicString<char, CharTraits<char>>;

    struct StackFrame
    {
        StackFrame* PreviousFrame      = nullptr;
        Pointer     InstructionPointer = nullptr;
    };
    using OnFrameCallback = void (*)(StackFrame* currentFrame);

    class Stacktrace
    {
      public:
        struct Symbol
        {
            char*                          Name;
            Pointer                        Address;

            const String                   Demangle() const;

            constexpr std::strong_ordering operator<=>(const Symbol& rhs) const
            {
                return Address <=> rhs.Address;
            }
        };

        Stacktrace() = default;
        Stacktrace(Pointer frameAddress, usize skipFrames = 0,
                   usize maxDepth = 32);

        ErrorOr<void> LoadSymbols(Vector<Symbol>&& symbols,
                                  PhysAddr lowestSymbolAddress  = 0x0000'0000,
                                  PhysAddr highestSymbolAddress = 0xffff'ffff)
        {
            m_Symbols              = Move(symbols);
            m_LowestSymbolAddress  = lowestSymbolAddress;
            m_HighestSymbolAddress = highestSymbolAddress;

            return {};
        }
        const Symbol*          GetSymbol(PhysAddr addr) const;

        constexpr StackFrame** begin() { return m_Frames.begin(); }
        constexpr StackFrame** end() { return m_Frames.end(); }

        static Stacktrace      GetCurrent();

      private:
        Vector<StackFrame*> m_Frames;

        Vector<Symbol>      m_Symbols;
        PhysAddr            m_LowestSymbolAddress  = 0x0000'0000;
        PhysAddr            m_HighestSymbolAddress = 0xffff'ffff;
    }; // namespace Stacktrace
}; // namespace Prism
