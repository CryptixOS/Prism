/*
 * Created by v1tr10l7 on 26.02.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

#include <Prism/Core/Compiler.hpp>
#include <Prism/Core/Config.hpp>
#include <Prism/Core/Types.hpp>

namespace Prism
{
    template <typename T>
    class DoublyLinkedList
    {
      public:
        struct Node
        {
            explicit Node(T&& value)
                : Value(value)
            {
            }

            T     Value;
            Node* m_Prev = nullptr;
            Node* m_Next = nullptr;
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
                m_Node = m_Node->m_Next;
                return *this;
            }
            Iterator& operator--()
            {
                m_Node = m_Node->m_Prev;
                return *this;
            }

            T& operator*() { return m_Node->Value; }
            T& operator->() { return &m_Node->Value; }

          private:
            Node* m_Node = nullptr;

            explicit Iterator(Node* node)
                : m_Node(node)
            {
            }

            friend class DoublyLinkedList;
        };
        using ReverseIteratorType = std::reverse_iterator<Iterator>;

        DoublyLinkedList()        = default;
        ~DoublyLinkedList() { Clear(); }

        T&             Front() { return m_Head; }
        const T&       Front() const { return m_Head; }

        T&             Back() { return m_Tail; }
        const T&       Back() const { return m_Tail; }

        Iterator       begin() { return Iterator(m_Head); }
        const Iterator begin() const { return Iterator(m_Head); }
        const Iterator cbegin() const PM_NOEXCEPT { return Iterator(m_Head); }

        Iterator       end() { return Iterator(m_Tail); }
        const Iterator end() const { return Iterator(m_Tail); }
        const Iterator cend() const PM_NOEXCEPT { return Iterator(m_Tail); }

        ReverseIteratorType       rbegin() { return m_Tail; }
        const ReverseIteratorType rbegin() const { return m_Tail; }
        const ReverseIteratorType crbegin() const PM_NOEXCEPT { return m_Tail; }

        ReverseIteratorType       rend() { return m_Head; }
        const ReverseIteratorType rend() const { return m_Head; }
        const ReverseIteratorType crend() const PM_NOEXCEPT { return m_Head; }

        bool                      IsEmpty() const { return m_Size == 0; }
        usize                     Size() const PM_NOEXCEPT { return m_Size; }
        usize                     MaxSize() const { return usize(-1); }

        void                      Clear()
        {
            Node* next    = m_Head->m_Next;
            Node* current = m_Head;
            while (current)
            {
                next = current->m_Next;
                delete current;
                current = next;
            }

            m_Head = nullptr;
            m_Tail = nullptr;
            m_Size = 0;
        }

        void PushBack(const T& value)
        {
            Node* newNode   = new Node(value);
            newNode->m_Next = nullptr;
            newNode->m_Prev = m_Tail;

            if (!m_Head)
            {
                m_Head = newNode;
                m_Tail = newNode;
            }
            if (m_Tail) m_Tail->m_Next = newNode;
        }
        void PushBack(T&& value)
        {
            Node* newNode   = new Node(std::move(value));
            newNode->m_Next = nullptr;
            newNode->m_Prev = m_Tail;

            if (!m_Head)
            {
                m_Head = newNode;
                m_Tail = newNode;
            }
            if (m_Tail) m_Tail->m_Next = newNode;
        }

      private:
        Node* m_Head = nullptr;
        Node* m_Tail = nullptr;
        usize m_Size = 0;
    };
}; // namespace Prism
