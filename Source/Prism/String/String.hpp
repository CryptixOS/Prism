/*
 * Created by v1tr10l7 on 27.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Containers/Vector.hpp>
#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Types.hpp>

#include <Prism/String/StringView.hpp>

namespace Prism
{
    enum class TrimMode
    {
        eLeft  = 1,
        eRight = 2,
        eBoth  = 3,
    };
    template <typename C, typename Traits = CharTraits<C>>
    class BasicString
    {

      public:
        using ViewType                 = BasicStringView<C, Traits>;
        using TraitsType               = Traits;
        using ValueType                = typename TraitsType::CharType;
        using SizeType                 = typename ViewType::SizeType;
        using DifferenceType           = ptrdiff;
        using ReferenceType            = ValueType&;
        using ConstReferenceType       = const ValueType&;
        using PointerType              = ValueType*;
        using ConstPointerType         = const ValueType*;
        using Iterator                 = ValueType*;
        using ConstIterator            = const ValueType*;
        using ReverseIterator          = Prism::ReverseIterator<Iterator>;
        using ConstReverseIterator     = Prism::ReverseIterator<ConstIterator>;

        constexpr static SizeType NPos = -1;

        explicit constexpr BasicString() PM_NOEXCEPT { ShortInit(); }
        constexpr BasicString(SizeType count, ValueType ch)
        {
            assert(count <= MaxSize());
            EnsureCapacity(count);
            Assign(count, ch);
            SetSize(count);

            Raw()[count] = 0;
        }
        template <typename InputIt>
        constexpr BasicString(InputIt first, InputIt last)
            : BasicString(first, Distance(first, last))
        {
        }
        constexpr BasicString(const ValueType* s, SizeType count = NPos)
        {
            if (!s)
            {
                ShortInit();
                return;
            }

            if (count == NPos) count = Traits::Length(s);
            ResizeIfNeededOverwrite(s, count);
        }
        constexpr BasicString(BasicStringView<C, Traits> str)
            : BasicString(str.Empty() ? nullptr : str.Raw(), str.Size())
        {
        }
        constexpr BasicString(const BasicString& other)
            : BasicString(BasicStringView{other.Raw(), other.Size()})
        {
        }
        constexpr BasicString(BasicString&& other)
        {
            m_Storage       = Move(other.m_Storage);
            other.m_Storage = {};
        }

        constexpr ~BasicString()
        {
            if (!IsLong()) return;

            delete[] m_Storage.Long.Data;
        }

        constexpr operator BasicStringView<C, Traits>()
        {
            return {Raw(), Size()};
        }

        constexpr BasicString& operator=(const BasicString& other)
        {
            if (other.Empty()) return *this;

            if (other == *this) return *this;
            ResizeIfNeededOverwrite(other.Raw(), other.Size());
            return *this;
        }
        constexpr BasicString& operator=(BasicString&& str)
        {
            if (IsLong()) delete[] Long().Data;
            m_Storage     = Move(str.m_Storage);
            str.m_Storage = {};

            return *this;
        }
        constexpr BasicString& operator=(const C* str)
        {
            if (!str || str == Raw()) return *this;
            ResizeIfNeededOverwrite(str, Traits::Length(str));

            return *this;
        }

        template <typename View>
            requires(IsConvertibleV<const View&, ViewType>
                     && !IsConvertibleV<const View&, const C*>)
        constexpr BasicString& operator=(const View& str)
        {
            ResizeIfNeededOverwrite(str.Raw(), str.Size());
            return *this;
        }
        constexpr BasicString& operator=(ValueType ch)
        {
            ShortInit();
            Raw()[0] = ch;
            SetSize(1);
            Raw()[Size()] = 0;

            return *this;
        }
        constexpr BasicString& operator=(NullType) = delete;

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
            return IsLong() ? Long().Data : Short();
        }
        constexpr const C* Raw() const PM_NOEXCEPT
        {
            return IsLong() ? Long().Data : Short();
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

        constexpr C*       end() PM_NOEXCEPT { return &Back() + 1; }
        constexpr const C* end() const PM_NOEXCEPT { return &Back() + 1; }
        constexpr const C* cend() const PM_NOEXCEPT { return &Back() + 1; }

        constexpr Prism::ReverseIterator<C*> rbegin() PM_NOEXCEPT
        {
            return &Back() + 1;
        }
        constexpr Prism::ReverseIterator<const C*> rbegin() const PM_NOEXCEPT
        {
            return &Back() + 1;
        }
        constexpr Prism::ReverseIterator<const C*> crbegin() const PM_NOEXCEPT
        {
            return &Back() + 1;
        }

        constexpr Prism::ReverseIterator<C*> rend() PM_NOEXCEPT
        {
            return &Front();
        }
        constexpr Prism::ReverseIterator<const C*> rend() const PM_NOEXCEPT
        {
            return &Front();
        }
        constexpr Prism::ReverseIterator<const C*> crend() const PM_NOEXCEPT
        {
            return &Front();
        }

        constexpr bool  Empty() const PM_NOEXCEPT { return Size() == 0; }
        constexpr usize Size() const PM_NOEXCEPT
        {
            return IsLong() ? Long().Size : m_Storage.Short.Size;
        }
        constexpr usize        MaxSize() const PM_NOEXCEPT { return usize(-1); }

        constexpr BasicString& Insert(SizeType pos, SizeType count,
                                      ValueType ch)
        {
            assert(Size() + count <= MaxSize());
            assert(pos <= Size());

            usize newSize = Max(pos + count, Size());

            EnsureCapacity(newSize);
            TraitsType::Assign(Raw() + pos, ch, count);

            SetSize(newSize);
            Raw()[Size()] = 0;

            return *this;
        }
        constexpr BasicString& Insert(SizeType pos, const ValueType* str)
        {
            assert(pos <= Size());
            assert(str);

            usize len     = Traits::Length(str);
            usize newSize = Max(pos + len, Size());

            assert(Size() + len <= MaxSize());

            EnsureCapacity(newSize);
            SetSize(newSize);
            Raw()[newSize] = 0;

            return *this;
        }

        constexpr void Reserve(usize newCapacity)
        {
            assert(newCapacity <= MaxSize());
            EnsureCapacity(newCapacity);
        }
        constexpr usize Capacity() const PM_NOEXCEPT
        {
            return IsLong() ? m_Storage.Long.Capacity : MIN_CAPACITY - 1;
        }
        constexpr void ShrinkToFit()
        {
            if (!IsLong()) return;
            Reallocate(Size(), true);
        }
        constexpr BasicString Trim(TrimMode mode = TrimMode::eBoth) const
        {
            if (Empty()) return BasicString<C, Traits>();

            usize start   = 0;
            usize end     = Size();

            auto  isSpace = [](C ch) constexpr { return ch == C(' '); };
            while ((mode == TrimMode::eLeft || mode == TrimMode::eBoth)
                   && start < end && isSpace(Raw()[start]))
                ++start;

            while ((mode == TrimMode::eRight || mode == TrimMode::eBoth)
                   && end > start && isSpace(Raw()[end - 1]))
                --end;

            return start >= Size() ? BasicString<C, Traits>("")
                                   : Substr(start, end - start);
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

            usize oldSize = Size();
            Resize(count);

            TraitsType::Assign(Raw() + oldSize, count - oldSize, ch);
            Raw()[Size()] = 0;
        }
        constexpr void Resize(SizeType count)
        {
            usize newSize = count;
            if (IsLong() || !FitsInSso(count)) Reallocate(count, true);

            for (usize i = Size(); i < count; i++) Raw()[i] = 0;
            SetSize(newSize);
            Raw()[newSize] = 0;
        }
        constexpr void Swap(BasicString& str) PM_NOEXCEPT
        {
            Prism::Swap(m_Storage, str.m_Storage);
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
            requires(IsConvertibleV<const Type&, ViewType>
                     && !IsConvertibleV<const Type&, const C*>)
        constexpr ViewType Find(const Type& t, ViewType pos = 0) const noexcept
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
            requires(IsConvertibleV<const Type&, ViewType>
                     && !IsConvertibleV<const Type&, const C*>)
        constexpr SizeType RFind(const Type& t,
                                 SizeType    pos = NPos) const noexcept
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
            requires(IsConvertibleV<const Type&, ViewType>
                     && !IsConvertibleV<const Type&, const C*>)
        constexpr SizeType FindFirstOf(const Type& t,
                                       SizeType    pos = 0) const noexcept
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
            return View().FindFirstNotOf(str, pos, count);
        }
        constexpr SizeType FindFirstNotOf(const ValueType* str,
                                          SizeType pos = 0) const PM_NOEXCEPT
        {
            return View().FindFirstNotOf(str, pos);
        }
        constexpr SizeType FindFirstNotOf(ValueType ch,
                                          SizeType  pos = 0) const PM_NOEXCEPT
        {
            return View().FindFirstNotOf(ch, pos);
        }
        template <typename T>
            requires(IsConvertibleV<const T&, ViewType>
                     && !IsConvertibleV<const T&, const C*>)
        constexpr SizeType FindFirstNotOf(const T& t,
                                          SizeType pos = 0) const noexcept
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
            requires(IsConvertibleV<const T&, ViewType>
                     && !IsConvertibleV<const T&, const C*>)
        constexpr SizeType FindLastOf(const T& t,
                                      SizeType pos = NPos) const noexcept
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
            requires(IsConvertibleV<const T&, ViewType>
                     && !IsConvertibleV<const T&, const C*>)
        constexpr SizeType FindLastNotOf(const T& t,
                                         SizeType pos = NPos) const PM_NOEXCEPT
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
            requires(IsConvertibleV<const T&, ViewType>
                     && !IsConvertibleV<const T&, const C*>)
        constexpr i32 Compare(const T& str) const PM_NOEXCEPT
        {
            return View().Compare(ViewType(str));
        }
        template <typename T>
            requires(IsConvertibleV<const T&, ViewType>
                     && !IsConvertibleV<const T&, const C*>)
        constexpr i32 Compare(SizeType pos, SizeType count,
                              const T& str) const PM_NOEXCEPT
        {
            return View().Compare(pos, count, ViewType(str));
        }
        template <typename T>
            requires(IsConvertibleV<const T&, ViewType>
                     && !IsConvertibleV<const T&, const C*>)
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
            assert(pos < Size());
            return BasicString(BasicStringView{Raw() + pos, count});
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
            if (rhs == *this)
            {
                usize size    = Size();
                usize newSize = 2 * size;
                EnsureCapacity(newSize);
                Traits::Move(Raw() + pos, Raw(), size);
                SetSize(newSize);
                Raw()[Size()] = 0;
                return *this;
            }

            usize newSize = Size() + rhs.Size();
            EnsureCapacity(newSize);
            SetSize(newSize);
            Raw()[Size()] = 0;

            TraitsType::Copy(Raw() + pos, rhs.Raw(), rhs.Size());
            return *this;
        }
        inline constexpr BasicString& operator+=(C ch)
        {
            auto  pos     = Size();
            usize newSize = Size() + 1;

            EnsureCapacity(newSize);
            Raw()[pos] = ch;

            SetSize(newSize);
            Raw()[newSize] = 0;

            return *this;
        }

        friend constexpr BasicString operator+(const BasicString& lhs,
                                               const BasicString& rhs)
        {
            BasicString string = "";
            string.Reserve(lhs.Size() + rhs.Size());
            string += lhs;
            string += rhs;

            return string;
        }
        friend constexpr BasicString operator+(const BasicString& lhs,
                                               const C*           rhs)
        {
            BasicString string = lhs;
            string += rhs;

            return string;
        }

      private:
        struct LongData
        {
            struct [[gnu::packed]]
            {
                usize IsLong   : 1;
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

        constexpr C*              Short() { return m_Storage.Short.Data; }
        constexpr const C*        Short() const { return m_Storage.Short.Data; }
        constexpr LongData&       Long() { return m_Storage.Long; }
        constexpr const LongData& Long() const { return m_Storage.Long; }
        constexpr void            SetSize(usize newSize)
        {
            if (IsLong()) m_Storage.Long.Size = newSize;
            else m_Storage.Short.Size = newSize;
        }

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
            constexpr usize ALIGNMENT = sizeof(ValueType*);
            newCapacity = (newCapacity + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
            if (newCapacity == Capacity()) return;

            usize newSize = Size();
            if (newCapacity < Size()) newSize = newCapacity;
            SetSize(newSize);

            C* newData = new C[newCapacity + 1];
            if (copyOld) TraitsType::Copy(newData, Raw(), Size());
            if (IsLong()) delete[] Long().Data;
            Long().Data            = newData;

            m_Storage.Short.IsLong = true;
            Long().Capacity        = newCapacity;
            Long().Size            = newSize;

            Raw()[Size()]          = 0;
        }
        constexpr void EnsureCapacity(usize newCapacity)
        {
            if (Capacity() >= newCapacity) return;
            Reallocate(newCapacity, true);
        }
        constexpr void ResizeIfNeededOverwrite(const C* string, usize length)
        {
            EnsureCapacity(length);
            SetSize(length);

            Traits::Copy(Raw(), string, length);
            Raw()[Size()] = 0;
        }

        constexpr void Assign(SizeType count, ValueType ch)
        {
            TraitsType::Assign(Raw(), count, ch);
        }
    };

    using String    = BasicString<char>;
    using U8String  = BasicString<char8_t>;
    using U16String = BasicString<char16_t>;
    using U32String = BasicString<char32_t>;
    using WString   = BasicString<wchar_t>;

    template <typename C, typename Traits>
    constexpr bool operator==(const BasicString<C, Traits>& lhs,
                              const BasicString<C, Traits>& rhs) PM_NOEXCEPT
    {
        return lhs.Compare(rhs) == 0;
    }
    template <typename C, typename Traits>
    constexpr bool operator==(const BasicString<C, Traits>&     lhs,
                              const BasicStringView<C, Traits>& rhs) PM_NOEXCEPT
    {
        return lhs.View().Compare(rhs) == 0;
    }

    template <typename C, typename Traits>
    constexpr bool operator==(const BasicString<C, Traits>& lhs, const C* rhs)
    {
        return lhs.Compare(rhs) == 0;
    }

    template <typename C, typename Traits>
    constexpr StrongOrdering
    operator<=>(const BasicString<C, Traits>& lhs,
                BasicStringView<C, Traits>    rhs) PM_NOEXCEPT
    {
        return lhs.View().Compare(rhs) <=> 0;
    }

    template <typename C, typename Traits>
    inline constexpr void
    Swap(BasicString<C, Traits>& lhs,
         BasicString<C, Traits>& rhs) noexcept(noexcept(lhs.Swap(rhs)))
    {
        lhs.Swap(rhs);
    }
    template <typename C, typename Traits>
    inline constexpr StrongOrdering
    operator<=>(const BasicString<C, Traits>& lhs,
                const BasicString<C, Traits>& rhs) noexcept
    {
        return lhs.Compare(rhs) <=> 0;
    }
    template <typename C, typename Traits>
    inline constexpr StrongOrdering
    operator<=>(const BasicString<C, Traits>& lhs, const C* rhs)
    {
        return lhs.Compare(rhs) <=> 0;
    }
    template <typename C, typename Traits = CharTraits<C>>
    constexpr BasicString<C, Traits>
    operator+(const BasicStringView<C, Traits> lhs, const C* rhs)
    {
        BasicString string = lhs;
        string += rhs;

        return string;
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

    // hash support
    template <>
    struct Hash<BasicString<char, CharTraits<char>>>
        : Detail::StringHashBase<char>
    {
    };

    template <>
    struct Hash<BasicString<char8_t, CharTraits<char8_t>>>
        : Detail::StringHashBase<char8_t>
    {
    };

    template <>
    struct Hash<BasicString<char16_t, CharTraits<char16_t>>>
        : Detail::StringHashBase<char16_t>
    {
    };

    template <>
    struct Hash<BasicString<char32_t, CharTraits<char32_t>>>
        : Detail::StringHashBase<char32_t>
    {
    };

    template <>
    struct Hash<BasicString<wchar_t, CharTraits<wchar_t>>>
        : Detail::StringHashBase<wchar_t>
    {
    };

}; // namespace Prism

#if PRISM_DISABLE_FMT == 0
template <>
struct fmt::formatter<Prism::String> : fmt::formatter<fmt::string_view>
{
    template <typename FormatContext>
    auto format(const Prism::String& src, FormatContext& ctx) const
    {
        return fmt::formatter<fmt::string_view>::format(
            fmt::format("{}", fmt::string_view(src.Raw(), src.Size())), ctx);
    }
};
#endif

#if PRISM_TARGET_CRYPTIX != 0
using Prism::BasicString;
using Prism::String;
using Prism::U16String;
using Prism::U32String;
using Prism::U8String;
using Prism::WString;
using Prism::Literals::operator""_s;
#endif
