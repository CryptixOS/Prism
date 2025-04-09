/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>
#include <Prism/String/StringUtils.hpp>
#include <Prism/String/StringView.hpp>

#include <concepts>

namespace Prism
{
    template <typename C, typename Traits = std::char_traits<C>>
    class BasicString
    {

      public:
        using ViewType                 = BasicStringView<C, Traits>;
        using TraitsType               = Traits;
        using ValueType                = typename TraitsType::char_type;
        using SizeType                 = typename ViewType::SizeType;
        using DifferenceType           = std::ptrdiff_t;
        using ReferenceType            = ValueType&;
        using ConstReferenceType       = const ValueType&;
        using PointerType              = ValueType*;
        using ConstPointerType         = const ValueType*;
        using Iterator                 = ValueType*;
        using ConstIterator            = const ValueType*;
        using ReverseIterator          = std::reverse_iterator<Iterator>;
        using ConstReverseIterator     = std::reverse_iterator<ConstIterator>;

        constexpr static SizeType NPos = -1;

        explicit constexpr BasicString() PM_NOEXCEPT { ShortInit(); }
        constexpr BasicString(SizeType count, ValueType ch)
        {
            assert(count <= MaxSize());

            if (count > Capacity()) Reallocate(count);

            TraitsType::assign(Raw(), count, ch);
        }
        template <typename InputIt>
        constexpr BasicString(InputIt first, InputIt last)
        {
            SizeType newSize = 1;
            for (auto it = first; it != last; it++) ++newSize;

            if (FitsInSso(newSize))
            {
                m_Storage.Short.Size   = newSize;
                m_Storage.Short.IsLong = false;
            }
            else
            {
                m_Storage.Long.Capacity = newSize;

                m_Storage.Long.Size     = newSize;
                m_Storage.Short.IsLong  = true;

                Reallocate(newSize, false);
            }

            usize i = 0;
            for (auto it = first; it != last; it++, i++) Raw()[i] = *it;
            Raw()[newSize - 1] = 0;
        }
        constexpr BasicString(const ValueType* s, SizeType count)
        {
            usize newSize = count + 1;
            if (FitsInSso(newSize))
            {
                m_Storage.Short.Size   = newSize - 1;
                m_Storage.Short.IsLong = false;
            }
            else
            {
                m_Storage.Long.Capacity = newSize;
                m_Storage.Long.Size     = newSize - 1;
                m_Storage.Short.IsLong  = true;
                Reallocate(newSize, false);
            }

            TraitsType::copy(Raw(), s, count);
            Raw()[newSize - 1] = 0;
        }
        constexpr BasicString(BasicStringView<C, Traits> str)
            : BasicString(const_cast<C*>(str.Raw()), str.Size())
        {
        }

        C& At(usize pos)
        {
            assert(pos < Size());
            return Raw()[pos];
        }
        const C& At(usize pos) const
        {
            assert(pos < Size());
            return Raw()[pos];
        }

        C& operator[](usize pos)
        {
            assert(pos < Size());
            return Raw()[pos];
        }
        const C& operator[](usize pos) const
        {
            assert(pos < Size());
            return Raw()[pos];
        }

        constexpr C&       Front() { return Raw()[0]; }
        constexpr const C& Front() const { return Raw()[0]; }

        constexpr C&       Back() { return Raw()[Size() - 1]; }
        constexpr const C& Back() const { return Raw()[Size() - 1]; }

        constexpr C*       Raw() PM_NOEXCEPT
        {
            return IsLong() ? m_Storage.Long.Data : m_Storage.Short.Data;
        }
        constexpr const C* Raw() const PM_NOEXCEPT
        {
            return IsLong() ? m_Storage.Long.Data : m_Storage.Short.Data;
        }

        constexpr BasicStringView<C> View() const PM_NOEXCEPT
        {
            return BasicStringView<C>(Raw(), Size());
        }
        constexpr operator BasicStringView<C>() const PM_NOEXCEPT
        {
            return View();
        }

        constexpr C*       begin() PM_NOEXCEPT { return &Front(); }
        constexpr const C* begin() const PM_NOEXCEPT { return &Front(); }
        constexpr const C* cbegin() const PM_NOEXCEPT { return &Front(); }

        constexpr C*       end() PM_NOEXCEPT { return &Back(); }
        constexpr const C* end() const PM_NOEXCEPT { return &Back(); }
        constexpr const C* cend() const PM_NOEXCEPT { return &Back(); }

        constexpr std::reverse_iterator<C*> rbegin() PM_NOEXCEPT
        {
            return &Back();
        }
        constexpr std::reverse_iterator<const C*> rbegin() const PM_NOEXCEPT
        {
            return &Back();
        }
        constexpr std::reverse_iterator<const C*> crbegin() const PM_NOEXCEPT
        {
            return &Back();
        }

        constexpr std::reverse_iterator<C*> rend() PM_NOEXCEPT
        {
            return &Front();
        }
        constexpr std::reverse_iterator<const C*> rend() const PM_NOEXCEPT
        {
            return &Front();
        }
        constexpr std::reverse_iterator<const C*> crend() const PM_NOEXCEPT
        {
            return &Front();
        }

        constexpr bool  Empty() const PM_NOEXCEPT { return Size() == 0; }
        constexpr usize Size() const PM_NOEXCEPT
        {
            return IsLong() ? m_Storage.Long.Size : m_Storage.Short.Size;
        }
        constexpr usize        MaxSize() const PM_NOEXCEPT { return usize(-1); }

        constexpr BasicString& Insert(SizeType pos, SizeType count,
                                      ValueType ch)
        {
            assert(Size() + count <= MaxSize());
            assert(pos <= Size());

            Insert(std::next(cbegin(), pos), count, ch);
            return *this;
        }
        constexpr BasicString& Insert(SizeType pos, const ValueType* str)
        {
            assert(pos <= Size());
            usize len = Traits::length(str);
            assert(Size() + len <= MaxSize());

            if (Size() != 0)
            {
                usize capacity = Capacity();
                usize size     = Size();
                usize newSize  = size + len;

                if (capacity < newSize) GrowTo(newSize);
                Traits::move(Raw() + pos + size, Raw() + pos, size - pos);
                Traits::copy(Raw() + pos, str, len);

                if (IsLong()) m_Storage.Long.Size = newSize;
                else m_Storage.Short.Size = newSize & 0x7f;
                Traits::assign(Raw()[Size()], 0);
            }
            InternalInsert(pos, str, len);

            return *this;
        }

        constexpr void Reserve(usize newCapacity)
        {
            assert(newCapacity <= MaxSize());
            if (newCapacity < Capacity()) return;

            newCapacity = std::max(newCapacity, Size());
            if (newCapacity == Capacity()) return;

            GrowTo(newCapacity);
        }
        constexpr usize Capacity() const PM_NOEXCEPT
        {
            return IsLong() ? m_Storage.Long.Capacity : MIN_CAPACITY;
        }
        constexpr void ShrinkToFit()
        {
            if (!IsLong()) return;
            Reallocate(Size(), true);
        }

        constexpr void Clear() PM_NOEXCEPT
        {
            if (IsLong())
            {
                m_Storage.Long.Size = 0;
                return;
            }

            m_Storage.Short.Size = 0;
        }
        constexpr void Resize(SizeType count, ValueType ch)
        {
            assert(Size() + count <= MaxSize());
            SizeType capacity = Capacity();
            SizeType size     = Size();
            SizeType newSize  = capacity + size;

            if (size < newSize)
            {
                if (capacity < newSize) GrowTo(newSize);
                TraitsType::assign(Raw() + size, count, ch);
            }

            if (IsLong()) m_Storage.Long.Size = newSize;
            else m_Storage.Short.Size = newSize;

            TraitsType::assign(Raw()[Size()], 0);
        }
        constexpr void Resize(SizeType count) { Resize(count, 0); }
        constexpr void Swap(BasicString& str) PM_NOEXCEPT
        {
            std::swap(m_Storage, str.m_Storage);
        }

        constexpr SizeType Find(const BasicString& str,
                                SizeType           pos = 0) const PM_NOEXCEPT
        {
            return View().Find(ViewType(str), pos);
        }
        constexpr SizeType Find(const ValueType* str, SizeType pos,
                                SizeType count) const PM_NOEXCEPT
        {
            return View().Find(str, pos, count);
        }
        constexpr SizeType Find(const ValueType* str,
                                SizeType         pos = 0) const PM_NOEXCEPT
        {
            return View().Find(str, pos);
        }
        constexpr SizeType Find(ValueType ch,
                                SizeType  pos = 0) const PM_NOEXCEPT
        {
            return View().Find(ch, pos);
        }
        template <typename Type>
            requires(std::is_convertible_v<const Type&, ViewType>
                     && !std::is_convertible_v<const Type&, const C*>)
        constexpr ViewType Find(const Type& t, ViewType pos = 0) const
            noexcept(std::is_nothrow_convertible_v<const Type&, ViewType>)
        {
            return View().Find(ViewType(t), pos);
        }

        constexpr SizeType RFind(const BasicString& str,
                                 SizeType pos = NPos) const PM_NOEXCEPT
        {
            return View().RFind(ViewType(str), pos);
        }
        constexpr SizeType RFind(const ValueType* str, SizeType pos,
                                 SizeType count) const PM_NOEXCEPT
        {
            return View().RFind(str, pos, count);
        }
        constexpr SizeType RFind(const ValueType* str,
                                 SizeType         pos = NPos) const PM_NOEXCEPT
        {
            return View().RFind(str, pos);
        }
        constexpr SizeType RFind(ValueType ch,
                                 SizeType  pos = NPos) const PM_NOEXCEPT
        {
            return View().RFind(ch, pos);
        }

        template <typename Type>
            requires(std::is_convertible_v<const Type&, ViewType>
                     && !std::is_convertible_v<const Type&, const C*>)
        constexpr SizeType rfind(const Type& t, SizeType pos = NPos) const
            noexcept(std::is_nothrow_convertible_v<const Type&, ViewType>)
        {
            return View().RFind(ViewType(t), pos);
        }

        constexpr SizeType FindFirstOf(const BasicString& str,
                                       SizeType pos = 0) const PM_NOEXCEPT
        {
            return View().FindFirstOf(ViewType(str), pos);
        }
        constexpr SizeType FindFirstOf(const ValueType* str, SizeType pos,
                                       SizeType count) const noexcept
        {
            return View().FindFirstOf(str, pos, count);
        }
        constexpr SizeType FindFirstOf(const ValueType* str,
                                       SizeType pos = 0) const PM_NOEXCEPT
        {
            return View().FindFirstOf(str, pos);
        }
        constexpr SizeType FindFirstOf(ValueType ch,
                                       SizeType  pos = 0) const PM_NOEXCEPT
        {
            return View().FindFirstOf(ch, pos);
        }
        template <typename Type>
            requires(std::is_convertible_v<const Type&, ViewType>
                     && !std::is_convertible_v<const Type&, const C*>)
        constexpr SizeType FindFirstOf(const Type& t, SizeType pos = 0) const
            noexcept(std::is_nothrow_convertible_v<const Type&, ViewType>)
        {
            return View().FindFirstOf(ViewType(t), pos);
        }
        constexpr SizeType FindFirstNotOf(const BasicString& str,
                                          SizeType pos = 0) const PM_NOEXCEPT
        {
            return View().FindFirstNotOf(ViewType(str), pos);
        }
        constexpr SizeType FindFirstNotOf(const ValueType* str, SizeType pos,
                                          SizeType count) const PM_NOEXCEPT
        {
            return View().FindLastNotOf(str, pos, count);
        }
        constexpr SizeType FindFirstNotOf(const ValueType* str,
                                          SizeType pos = 0) const PM_NOEXCEPT
        {
            return View().FindLastNotOf(str, pos);
        }
        constexpr SizeType FindFirstNotOf(ValueType ch,
                                          SizeType  pos = 0) const PM_NOEXCEPT
        {
            return View().FindFirstNotOf(ch, pos);
        }
        template <typename T>
            requires(std::is_convertible_v<const T&, ViewType>
                     && !std::is_convertible_v<const T&, const C*>)
        constexpr SizeType FindFirstNotOf(const T& t, SizeType pos = 0) const
            noexcept(std::is_nothrow_convertible_v<const T&, ViewType>)
        {
            return View().FindFirstNotOf(ViewType(t), pos);
        }
        constexpr SizeType FindLastOf(const BasicString& str,
                                      SizeType pos = NPos) const PM_NOEXCEPT
        {
            return View().FindLastOf(ViewType(str), pos);
        }
        constexpr SizeType FindLastOf(const ValueType* str, SizeType pos,
                                      SizeType count) const PM_NOEXCEPT
        {
            return View().FindLastOf(str, pos, count);
        }

        constexpr SizeType FindLastOf(const ValueType* str,
                                      SizeType pos = NPos) const PM_NOEXCEPT
        {
            return View().FindLastOf(str, pos);
        }

        constexpr SizeType FindLastOf(ValueType ch,
                                      SizeType  pos = NPos) const PM_NOEXCEPT
        {
            return View().FindLastOf(ch, pos);
        }
        template <typename T>
            requires(std::is_convertible_v<const T&, ViewType>
                     && !std::is_convertible_v<const T&, const C*>)
        constexpr SizeType FindLastOf(const T& t, SizeType pos = NPos) const
            noexcept(std::is_nothrow_convertible_v<const T&, ViewType>)
        {
            return View().FindLastOf(ViewType(t), pos);
        }

        constexpr SizeType FindLastNotOf(const BasicString& str,
                                         SizeType pos = NPos) const PM_NOEXCEPT
        {
            return View().FindLastNotOf(ViewType(str), pos);
        }
        constexpr SizeType FindLastNotOf(const ValueType* str, SizeType pos,
                                         SizeType count) const PM_NOEXCEPT
        {
            return View().FindLastNotOf(str, pos, count);
        }
        constexpr SizeType FindLastNotOf(const ValueType* str,
                                         SizeType pos = NPos) const PM_NOEXCEPT
        {
            return View().FindLastNotOf(str, pos);
        }
        constexpr SizeType FindLastNotOf(ValueType ch,
                                         SizeType  pos = NPos) const PM_NOEXCEPT
        {
            return View().FindLastNotOf(ch, pos);
        }
        template <typename T>
            requires(std::is_convertible_v<const T&, ViewType>
                     && !std::is_convertible_v<const T&, const C*>)
        constexpr SizeType FindLastNotOf(const T& t, SizeType pos = NPos) const
            PM_NOEXCEPT(std::is_nothrow_convertible_v<const T&, ViewType>)
        {
            return View().FindLastNotOf(ViewType(t), pos);
        }

        constexpr i32 Compare(const BasicString& str) const PM_NOEXCEPT
        {
            return View().Compare(str.View());
        }
        constexpr i32 Compare(SizeType pos, SizeType count,
                              const BasicString& str) const
        {
            return View().Compare(pos, count, str.View());
        }
        constexpr i32 Compare(SizeType pos1, SizeType count1,
                              const BasicString& str, SizeType pos2,
                              SizeType count2 = NPos) const
        {
            return View().Compare(pos1, count1, str.View(), pos2, count2);
        }
        constexpr i32 Compare(const ValueType* str) const
        {
            return View().Compare(str);
        }
        constexpr i32 Compare(SizeType pos, SizeType count,
                              const ValueType* str) const
        {
            return View().Compare(pos, count, str);
        }
        constexpr i32 Compare(SizeType pos, SizeType count1,
                              const ValueType* str, SizeType count2) const
        {
            return View().Compare(pos, count1, str, count2);
        }
        template <typename T>
            requires(std::is_convertible_v<const T&, ViewType>
                     && !std::is_convertible_v<const T&, const C*>)
        constexpr i32 Compare(const T& str) const PM_NOEXCEPT(
            PM_NOEXCEPT(std::is_nothrow_convertible_v<const T&, ViewType>))
        {
            return View().Compare(ViewType(str));
        }
        template <typename T>
            requires(std::is_convertible_v<const T&, ViewType>
                     && !std::is_convertible_v<const T&, const C*>)
        constexpr i32 Compare(SizeType pos, SizeType count, const T& str) const
            PM_NOEXCEPT(
                PM_NOEXCEPT(std::is_nothrow_convertible_v<const T&, ViewType>))
        {
            return View().Compare(pos, count, ViewType(str));
        }
        template <typename T>
            requires(std::is_convertible_v<const T&, ViewType>
                     && !std::is_convertible_v<const T&, const C*>)
        constexpr i32 Compare(SizeType pos1, SizeType count1, const T& str,
                              SizeType pos2, SizeType count2 = NPos) const
        {
            return View().Compare(pos1, count1, ViewType(str), pos2, count2);
        }

        constexpr bool StartsWith(ViewType sv) const PM_NOEXCEPT
        {
            return View().StartsWith(sv);
        }
        constexpr bool StartsWith(C ch) const PM_NOEXCEPT
        {
            return View().StartsWith(ch);
        }
        constexpr bool StartsWith(const C* str) const PM_NOEXCEPT
        {
            return View().StartsWith(str);
        }

        constexpr bool EndsWith(ViewType sv) const PM_NOEXCEPT
        {
            return View().EndsWith(sv);
        }
        constexpr bool EndsWith(C ch) const PM_NOEXCEPT
        {
            return View().EndsWith(ch);
        }
        constexpr bool EndsWith(const C* str) const PM_NOEXCEPT
        {
            return View().EndsWith(str);
        }

        constexpr BasicString Substr(SizeType pos   = 0,
                                     SizeType count = NPos) const
        {
            assert(pos <= Size());
            return BasicString(*this, pos, count);
        }
        constexpr SizeType Copy(ValueType* str, SizeType count,
                                SizeType pos = 0) const
        {
            assert(pos <= Size());
            return View().Copy(str, count, pos);
        }

        inline constexpr BasicString<C, Traits>&
        operator+=(BasicStringView<C, Traits> rhs)
        {
            auto pos = Size();
            GrowTo(Size() + rhs.Size() + 1);

            for (auto c : rhs) Raw()[pos++] = c;
            Raw()[pos] = 0;

            return *this;
        }

        template <typename C2, typename Traits2>
        friend BasicString<C2, Traits2>
        operator+(BasicStringView<C2, Traits2> lhs,
                  BasicStringView<C2, Traits2> rhs);
        template <typename C2, typename Traits2>
        friend BasicString<C2, Traits2>
        operator+(BasicStringView<C2, Traits2> lhs, const C* rhs);

      private:
        struct LongData
        {
            struct [[gnu::packed]]
            {
                usize IsLong : 1;
                usize Capacity : sizeof(usize) * __CHAR_BIT__ - 1;
            };
            usize Size = 0;
            C*    Data = nullptr;
        };

        static constexpr usize MIN_CAPACITY
            = (sizeof(LongData) - 1) / sizeof(C) > 2
                ? (sizeof(LongData) - 1) / sizeof(C)
                : 2;
        struct ShortData
        {
            struct [[gnu::packed]]
            {
                unsigned char IsLong : 1;
                unsigned char Size   : 7;
            };
            char Padding[sizeof(C) - 1];
            C    Data[MIN_CAPACITY];
        };

        union Storage
        {
            LongData  Long;
            ShortData Short{{false, 0}, {}, {}};
        } m_Storage;

        constexpr static bool FitsInSso(usize size)
        {
            return size < MIN_CAPACITY;
        }

        constexpr void Initialize(usize size)
        {
            if (FitsInSso(size)) return ShortInit();
            LongInit();
        }

        constexpr void LongInit()
        {
            m_Storage.Long.IsLong   = true;
            m_Storage.Long.Data     = nullptr;
            m_Storage.Long.Size     = 0;
            m_Storage.Long.Capacity = 0;
        }
        constexpr void ShortInit()
        {
            if (IsLong() && m_Storage.Long.Data)
            {
                delete[] m_Storage.Long.Data;
                m_Storage.Long.Data     = nullptr;
                m_Storage.Long.Size     = 0;
                m_Storage.Long.Capacity = 0;
                m_Storage.Long.IsLong   = false;
            }

            m_Storage.Short.IsLong = false;
            m_Storage.Short.Size   = 0;
        }

        constexpr bool IsLong() const PM_NOEXCEPT
        {
            return m_Storage.Short.IsLong;
        }

        constexpr void Reallocate(usize newCapacity, bool copyOld)
        {
            if (newCapacity == m_Storage.Long.Capacity) return;

            usize oldSize = m_Storage.Long.Size;
            // usize oldCapacity = m_Storage.Long.Capacity;
            C*    oldData = m_Storage.Long.Data;

            usize newSize = std::min(newCapacity, oldSize);
            C*    newData = new C[newCapacity + 1];

            if (!oldData)
            {
                if (oldSize != 0 && copyOld)
                    std::char_traits<C>::copy(newData, oldData, newSize);
                delete[] oldData;
            }

            m_Storage.Long.Size     = newSize;
            m_Storage.Long.Data     = newData;
            m_Storage.Long.Capacity = newCapacity;
            ;
        }
        constexpr void GrowTo(usize newCapacity)
        {
            if (IsLong())
            {
                Reallocate(newCapacity, true);
                return;
            }

            C*    newData = new C[newCapacity + 1];
            usize newSize = m_Storage.Short.Size;

            std::char_traits<C>::copy(newData, m_Storage.Short.Data, newSize);
            std::char_traits<C>::assign(newData[newSize], 0);

            LongInit();
            m_Storage.Long.Data     = newData;
            m_Storage.Long.Size     = newSize;
            m_Storage.Long.Capacity = newCapacity;
        }

        constexpr void Assign(SizeType count, ValueType ch)
        {
            if (FitsInSso(count))
            {
                if (IsLong()) ShortInit();

                m_Storage.Short.Size = count;
            }
        }
    };

    using String    = BasicString<char>;
    using U8String  = BasicString<char8_t>;
    using U16String = BasicString<char16_t>;
    using U32String = BasicString<char32_t>;
    using WString   = BasicString<wchar_t>;

    template <typename C, typename Traits>
    inline BasicString<C, Traits> operator+(BasicStringView<C, Traits> lhs,
                                            BasicStringView<C, Traits> rhs)
    {
        BasicString<C, Traits> string;
        string.Resize(lhs.Size() + rhs.Size() + 1);
        for (usize i = 0; auto c : lhs) string[i++] = c;
        for (usize i = lhs.Size(); auto c : rhs) string[i++] = c;
        string[lhs.Size() + rhs.Size()] = 0;

        return std::move(string);
    }
    template <typename C, typename Traits>
    BasicString<C, Traits> operator+(BasicStringView<C, Traits> lhs,
                                     const C*                   rhs)
    {
        BasicString<C, Traits> string;
        usize                  len = Traits::length(rhs);
        string.Resize(lhs.Size() + len + 1);

        for (usize i = 0; auto c : lhs) string[i++] = c;

        C* src = const_cast<C*>(rhs);
        for (usize i = lhs.Size(); *src++ != '\0';) string[i++] = *src;
        string[lhs.Size() + len] = 0;

        return std::move(string);
    }

    template <typename C, typename Traits>
    inline constexpr void
    Swap(BasicString<C, Traits>& lhs,
         BasicString<C, Traits>& rhs) noexcept(noexcept(lhs.Swap(rhs)))
    {
        lhs.Swap(rhs);
    }

    template <typename C, typename Traits>
    inline constexpr std::strong_ordering
    operator<=>(const BasicString<C, Traits>& lhs,
                const BasicString<C, Traits>& rhs) noexcept
    {
        return lhs.Compare(rhs) <=> 0;
    }

    template <typename C, typename Traits>
    inline constexpr std::strong_ordering
    operator<=>(const BasicString<C, Traits>& lhs, const C* rhs)
    {
        return lhs.Compare(rhs) <=> 0;
    }

    namespace Detail
    {
        inline constexpr u64 MurmurHash2_64A(const void* key, u64 size,
                                             u64 seed)
        {
            const u64  m    = 0xc6a4a7935bd1e995;
            const int  r    = 47;

            u64        h    = seed ^ (size * m);

            const u64* data = static_cast<const uint64_t*>(key);
            const u64* end  = data + (size / 8);

            while (data != end)
            {
                u64 k = 0;
                k     = *(data++);

                k *= m;
                k ^= k >> r;
                k *= m;

                h ^= k;
                h *= m;
            }

            auto data2 = static_cast<const u8*>(static_cast<const void*>(data));

            switch (size & 7)
            {
                case 7: h ^= static_cast<u64>(data2[6]) << 48; [[fallthrough]];
                case 6: h ^= static_cast<u64>(data2[5]) << 40; [[fallthrough]];
                case 5: h ^= static_cast<u64>(data2[4]) << 32; [[fallthrough]];
                case 4: h ^= static_cast<u64>(data2[3]) << 24; [[fallthrough]];
                case 3: h ^= static_cast<u64>(data2[2]) << 16; [[fallthrough]];
                case 2: h ^= static_cast<u64>(data2[1]) << 8; [[fallthrough]];
                case 1: h ^= static_cast<u64>(data2[0]); h *= m;
            };

            h ^= h >> r;
            h *= m;
            h ^= h >> r;

            return h;
        }

        template <typename C,
                  typename String = BasicString<C, std::char_traits<C>>>
        struct StringHashBase
        {
            [[nodiscard]] constexpr usize
            operator()(const String& str) const PM_NOEXCEPT
            {
                return MarmurHash2_64A(str.Raw(), str.Size() * sizeof(C),
                                       0xc70f6907ul);
            }
        };
    }; // namespace Detail

    inline String ToString(i32 value)
    {
        return String(StringUtils::ToString(value, 10),
                      StringUtils::GetDigitCount(value));
    }

    namespace Literals
    {
        constexpr inline String operator""_s(const char* str, usize size)
        {
            return String(str, size);
        }
        constexpr inline U8String operator""_s(const char8_t* str, usize size)
        {
            return U8String(str, size);
        }
        constexpr inline U16String operator""_s(const char16_t* str, usize size)
        {
            return U16String(str, size);
        }
        constexpr inline U32String operator""_s(const char32_t* str, usize size)
        {
            return U32String(str, size);
        }
        constexpr inline WString operator""_s(const wchar_t* str, usize size)
        {
            return WString(str, size);
        }
    }; // namespace Literals

}; // namespace Prism
// hash support
template <>
struct std::hash<Prism::BasicString<char, std::char_traits<char>>>
    : Prism::Detail::StringHashBase<char>
{
};

template <>
struct std::hash<Prism::BasicString<char8_t, std::char_traits<char8_t>>>
    : Prism::Detail::StringHashBase<char8_t>
{
};

template <>
struct std::hash<Prism::BasicString<char16_t, std::char_traits<char16_t>>>
    : Prism::Detail::StringHashBase<char16_t>
{
};

template <>
struct std::hash<Prism::BasicString<char32_t, std::char_traits<char32_t>>>
    : Prism::Detail::StringHashBase<char32_t>
{
};

template <>
struct std::hash<Prism::BasicString<wchar_t, std::char_traits<wchar_t>>>
    : Prism::Detail::StringHashBase<wchar_t>
{
};

#if PRISM_TARGET_CRYPTIX == 1
using Prism::BasicString;
#endif
