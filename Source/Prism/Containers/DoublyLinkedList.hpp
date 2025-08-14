/*
 * Created by v1tr10l7 on 26.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Core.hpp>
#include <Prism/Core/Iterator.hpp>

#include <cassert>

namespace Prism
{
    template <typename T>
    class DoublyLinkedList
    {
      public:
        struct Node
        {
            explicit Node(const T& value)
                : Value(value)
            {
            }
            explicit Node(T&& value)
                : Value(Move(value))
            {
            }

            T     Value;
            Node* Prev = nullptr;
            Node* Next = nullptr;
        };

        struct Iterator
        {
            bool operator!=(const Iterator& other) const
            {
                return m_Node != other.m_Node;
            }
            bool operator==(const Iterator& other) const
            {
                return m_Node == other.m_Node;
            }
            Iterator& operator++()
            {
                m_Node = m_Node->Next;
                return *this;
            }
            Iterator& operator--()
            {
                m_Node = m_Node->Prev;
                return *this;
            }

            T&              operator*() { return m_Node->Value; }
            T*              operator->() { return &m_Node->Value; }
            inline bool     IsEnd() const { return !m_Node; }
            static Iterator UniversalEnd() { return Iterator(nullptr); }

          private:
            Node* m_Node = nullptr;

            explicit Iterator(Node* node)
                : m_Node(node)
            {
            }

            friend class DoublyLinkedList;
        };
        using ReverseIteratorType = Prism::ReverseIterator<Iterator>;

        DoublyLinkedList()        = default;
        ~DoublyLinkedList() { Clear(); }

        constexpr DoublyLinkedList(const DoublyLinkedList& other)
        {
            for (const auto& element : other) EmplaceBack(element);
        }
        constexpr DoublyLinkedList(DoublyLinkedList&& other)
        {
            m_Head       = other.m_Head;
            m_Tail       = other.m_Tail;
            m_Size       = other.m_Size;

            other.m_Head = nullptr;
            other.m_Tail = nullptr;
            other.m_Size = 0;
        }

        constexpr DoublyLinkedList& operator=(const DoublyLinkedList& other)
        {
            if (m_Head) Clear();
            for (const auto& element : other) EmplaceBack(element);

            return *this;
        }
        constexpr DoublyLinkedList& operator=(DoublyLinkedList&& other)
        {
            if (m_Head) Clear();
            m_Head       = other.m_Head;
            m_Tail       = other.m_Tail;
            m_Size       = other.m_Size;

            other.m_Head = nullptr;
            other.m_Tail = nullptr;
            other.m_Size = 0;
            return *this;
        }

        inline T& operator[](usize index)
        {
            assert(index < m_Size);

            Node* node = index < m_Size / 2 ? m_Head : m_Tail;
            if (node == m_Head)
                while (index--) node = node->Next;
            else
                for (usize i = m_Size - 1; i > index; --i) node = node->Prev;

            return node->Value;
        }

        T&             Head() { return m_Head->Value; }
        const T&       Head() const { return m_Head->Value; }

        T&             Tail() { return m_Tail->Value; }
        const T&       Tail() const { return m_Tail->Value; }

        Iterator       begin() { return Iterator(m_Head); }
        const Iterator begin() const { return Iterator(m_Head); }
        const Iterator cbegin() const PM_NOEXCEPT { return Iterator(m_Head); }

        Iterator       end() { return Iterator::UniversalEnd(); }
        const Iterator end() const { return Iterator::UniversalEnd(); }
        const Iterator cend() const PM_NOEXCEPT
        {
            return Iterator::UniversalEnd();
        }

        ReverseIteratorType       rbegin() { return Iterator::UniversalEnd(); }
        const ReverseIteratorType rbegin() const
        {
            return Iterator::UniversalEnd();
        }
        const ReverseIteratorType crbegin() const PM_NOEXCEPT
        {
            return Iterator::UniversalEnd();
        }

        ReverseIteratorType       rend() { return Iterator(m_Head); }
        const ReverseIteratorType rend() const { return Iterator(m_Head); }
        const ReverseIteratorType crend() const PM_NOEXCEPT
        {
            return Iterator(m_Head);
        }

        inline constexpr bool  Empty() const { return m_Size == 0; }
        inline constexpr usize Size() const PM_NOEXCEPT { return m_Size; }
        inline constexpr usize MaxSize() const { return usize(-1); }

        void                   Clear()
        {
            Node* current = m_Head;
            Node* next    = nullptr;
            while (current)
            {
                next = current->Next;
                delete current;
                current = next;
            }

            m_Head = nullptr;
            m_Tail = nullptr;
            m_Size = 0;
        }

        template <typename... Args>
        inline T& EmplaceBack(Args&&... args)
        {
            PushBack(T(Forward<Args>(args)...));

            return Tail();
        }
        inline Iterator PushBack(const T& value)
        {
            Node* node = new Node(value);
            node->Next = nullptr;
            node->Prev = m_Tail;

            if (m_Tail) m_Tail->Next = node;
            else m_Head = node; // List was empty

            m_Tail = node;
            ++m_Size;

            return Iterator(node);
        }
        inline Iterator PushBack(T&& value)
        {
            Node* node = new Node(value);
            node->Next = nullptr;
            node->Prev = m_Tail;

            if (m_Tail) m_Tail->Next = node;
            else m_Head = node; // List was empty

            m_Tail = node;
            ++m_Size;

            return Iterator(node);
        }

        inline Iterator PushFront(const T& value)
        {
            Node* newNode = new Node(value);
            newNode->Next = m_Head;
            newNode->Prev = nullptr;

            if (m_Head) m_Head->Prev = newNode;
            else m_Tail = newNode; // List was empty

            m_Head = newNode;
            ++m_Size;
            return Iterator(newNode);
        }
        inline Iterator PushFront(T&& value)
        {
            Node* newNode = new Node(value);
            newNode->Next = m_Head;
            newNode->Prev = nullptr;

            if (m_Head) m_Head->Prev = newNode;
            else m_Tail = newNode; // List was empty

            m_Head = newNode;
            ++m_Size;

            return Iterator(newNode);
        }

        inline T PopFrontElement()
        {
            auto front = m_Head;
            auto value = front->Value;

            Erase(Iterator(front));
            return value;
        }
        inline T PopBackElement()
        {
            auto back  = m_Tail;
            auto value = back->Value;

            Erase(Iterator(back));
            return value;
        }

        inline Iterator Erase(Iterator it)
        {
            auto node = it.m_Node;
            auto prev = node->Prev;
            auto next = node->Next;

            if (node == m_Head) m_Head = prev;
            else if (node == m_Tail) m_Tail = next;

            if (prev) prev->Next = next;
            else m_Head = next;

            if (next) next->Prev = prev;
            else m_Tail = prev;

            delete node;

            --m_Size;
            return next ? Iterator(next) : Iterator::UniversalEnd();
        }

      private:
        Node* m_Head = nullptr;
        Node* m_Tail = nullptr;
        usize m_Size = 0;
    };
}; // namespace Prism

#if PRISM_TARGET_CRYPTIX == 1
using Prism::DoublyLinkedList;
#endif
