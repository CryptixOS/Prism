/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

// Prism containers vs their std counterparts. See Common/Bench.hpp for
// the flags and how to read the output.

#include "../Common/Bench.hpp"

#include <bitset>
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

import Prism.Containers.Bitmap;
import Prism.Containers.Deque;
import Prism.Containers.DoublyLinkedList;
import Prism.Containers.IntrusiveListV3;
import Prism.Containers.Queue;
import Prism.Containers.RedBlackTree;
import Prism.Containers.Set;
import Prism.Containers.Stack;
import Prism.Containers.UnorderedMap;
import Prism.Containers.Vector;
import Prism.String.String;

using Bench::Name;
using Bench::Opaque;
using Bench::u64;
using Bench::Runner;

static std::vector<u64> RandomKeys(size_t n, u64 seed = 1)
{
    Bench::Rng       rng(seed);
    std::vector<u64> keys(n);
    for (auto& k : keys) k = rng.Next() >> 16;
    return keys;
}
// n distinct random keys.
static std::vector<u64> UniqueKeys(size_t n, u64 seed = 1)
{
    std::vector<u64> keys(n);
    for (size_t i = 0; i < n; ++i) keys[i] = i * 2 + 1;
    Bench::Rng rng(seed);
    for (size_t i = n; i > 1; --i) std::swap(keys[i - 1], keys[rng.Below(i)]);
    return keys;
}
static std::string KeyString(u64 key)
{
    char buffer[40];
    std::snprintf(buffer, sizeof(buffer), "bench/key/%llu",
                  static_cast<unsigned long long>(key));
    return buffer;
}

static const size_t kSizes[] = {16, 1000, 100000};

// ============================================================================
// Vector

static void RegisterVector()
{
    for (size_t n : kSizes)
    {
        Bench::Add("Vector", Name("PushBack/u64", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::Vector<u64> v;
                    for (size_t i = 0; i < Opaque(n); ++i) v.PushBack(i);
                    return u64(v.Size() + v[n / 2]);
                },
                [n] {
                    std::vector<u64> v;
                    for (size_t i = 0; i < Opaque(n); ++i) v.push_back(i);
                    return u64(v.size() + v[n / 2]);
                });
        });
        Bench::Add("Vector", Name("ReservePushBack/u64", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::Vector<u64> v;
                    v.Reserve(n);
                    for (size_t i = 0; i < Opaque(n); ++i) v.PushBack(i);
                    return u64(v.Size() + v[n / 2]);
                },
                [n] {
                    std::vector<u64> v;
                    v.reserve(n);
                    for (size_t i = 0; i < Opaque(n); ++i) v.push_back(i);
                    return u64(v.size() + v[n / 2]);
                });
        });
        Bench::Add("Vector", Name("PushBack/string", n), [n](Runner& r) {
            std::vector<std::string> src;
            for (size_t i = 0; i < n; ++i) src.push_back(KeyString(i * 7919));
            r.Compare(
                n,
                [&] {
                    Prism::Vector<Prism::String> v;
                    for (auto& s : src) v.PushBack(Prism::String(s.c_str()));
                    return u64(v.Size() + v[n / 2].Size());
                },
                [&] {
                    std::vector<std::string> v;
                    for (auto& s : src) v.push_back(std::string(s.c_str()));
                    return u64(v.size() + v[n / 2].size());
                });
        });
        Bench::Add("Vector", Name("Iterate/u64", n), [n](Runner& r) {
            auto               keys = RandomKeys(n);
            Prism::Vector<u64> pv(keys.begin(), keys.end());
            std::vector<u64>   sv(keys.begin(), keys.end());
            r.Compare(
                n,
                [&] {
                    u64 sum = 0;
                    for (u64 x : pv) sum += x;
                    return sum;
                },
                [&] {
                    u64 sum = 0;
                    for (u64 x : sv) sum += x;
                    return sum;
                });
        });
        Bench::Add("Vector", Name("Copy/u64", n), [n](Runner& r) {
            auto               keys = RandomKeys(n);
            Prism::Vector<u64> pv(keys.begin(), keys.end());
            std::vector<u64>   sv(keys.begin(), keys.end());
            r.Compare(
                n,
                [&] {
                    Prism::Vector<u64> copy(pv);
                    return u64(copy[n - 1]);
                },
                [&] {
                    std::vector<u64> copy(sv);
                    return u64(copy[n - 1]);
                });
        });
        Bench::Add("Vector", Name("Resize/u64", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::Vector<u64> v;
                    v.Resize(Opaque(n));
                    return u64(v.Size() + v[n - 1]);
                },
                [n] {
                    std::vector<u64> v;
                    v.resize(Opaque(n));
                    return u64(v.size() + v[n - 1]);
                });
        });
    }
    for (size_t n : {16, 1000, 10000})
    {
        // Inserting at the front is O(n) per element: measures the shifting.
        Bench::Add("Vector", Name("InsertFront/u64", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::Vector<u64> v;
                    for (size_t i = 0; i < Opaque(n); ++i) v.Insert(v.begin(), i);
                    return u64(v.Size() + v[0]);
                },
                [n] {
                    std::vector<u64> v;
                    for (size_t i = 0; i < Opaque(n); ++i) v.insert(v.begin(), i);
                    return u64(v.size() + v[0]);
                });
        });
        Bench::Add("Vector", Name("EraseFront/u64", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::Vector<u64> v;
                    v.Resize(n);
                    u64 sum = 0;
                    while (!v.Empty())
                    {
                        sum += v.Size();
                        v.Erase(v.begin());
                    }
                    return sum;
                },
                [n] {
                    std::vector<u64> v;
                    v.resize(n);
                    u64 sum = 0;
                    while (!v.empty())
                    {
                        sum += v.size();
                        v.erase(v.begin());
                    }
                    return sum;
                });
        });
    }
}

// ============================================================================
// Deque, Queue, Stack, CircularQueue

static void RegisterQueues()
{
    for (size_t n : kSizes)
    {
        Bench::Add("Deque", Name("PushBack", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::Deque<u64> d;
                    for (size_t i = 0; i < Opaque(n); ++i) d.PushBack(i);
                    return u64(d.Size() + d.Back());
                },
                [n] {
                    std::deque<u64> d;
                    for (size_t i = 0; i < Opaque(n); ++i) d.push_back(i);
                    return u64(d.size() + d.back());
                });
        });
        Bench::Add("Deque", Name("PushFront", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::Deque<u64> d;
                    for (size_t i = 0; i < Opaque(n); ++i) d.PushFront(i);
                    return u64(d.Size() + d.Front());
                },
                [n] {
                    std::deque<u64> d;
                    for (size_t i = 0; i < Opaque(n); ++i) d.push_front(i);
                    return u64(d.size() + d.front());
                });
        });
        Bench::Add("Deque", Name("PushBackPopFront", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::Deque<u64> d;
                    u64               sum = 0;
                    for (size_t i = 0; i < Opaque(n); ++i) d.PushBack(i);
                    while (!d.Empty())
                    {
                        sum += d.Front();
                        d.PopFront();
                    }
                    return sum;
                },
                [n] {
                    std::deque<u64> d;
                    u64             sum = 0;
                    for (size_t i = 0; i < Opaque(n); ++i) d.push_back(i);
                    while (!d.empty())
                    {
                        sum += d.front();
                        d.pop_front();
                    }
                    return sum;
                });
        });
        Bench::Add("Deque", Name("Iterate", n), [n](Runner& r) {
            Prism::Deque<u64> pd;
            std::deque<u64>   sd;
            for (u64 k : RandomKeys(n))
            {
                pd.PushBack(k);
                sd.push_back(k);
            }
            r.Compare(
                n,
                [&] {
                    u64 sum = 0;
                    for (u64 x : pd) sum += x;
                    return sum;
                },
                [&] {
                    u64 sum = 0;
                    for (u64 x : sd) sum += x;
                    return sum;
                });
        });

        Bench::Add("Queue", Name("PushPop", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::Queue<u64> q;
                    u64               sum = 0;
                    for (size_t i = 0; i < Opaque(n); ++i) q.Push(i);
                    while (!q.Empty())
                    {
                        sum += q.Front();
                        q.Pop();
                    }
                    return sum;
                },
                [n] {
                    std::queue<u64> q;
                    u64             sum = 0;
                    for (size_t i = 0; i < Opaque(n); ++i) q.push(i);
                    while (!q.empty())
                    {
                        sum += q.front();
                        q.pop();
                    }
                    return sum;
                });
        });
        Bench::Add("Stack", Name("PushPop", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::Stack<u64> s;
                    u64               sum = 0;
                    for (size_t i = 0; i < Opaque(n); ++i) s.Push(i);
                    while (!s.Empty())
                    {
                        sum += s.Back();
                        s.Pop();
                    }
                    return sum;
                },
                [n] {
                    std::stack<u64> s;
                    u64             sum = 0;
                    for (size_t i = 0; i < Opaque(n); ++i) s.push(i);
                    while (!s.empty())
                    {
                        sum += s.top();
                        s.pop();
                    }
                    return sum;
                });
        });
    }

    // Prism::CircularQueue is not benchmarked: it does not compile once
    // instantiated (m_Storage is a u8[] indexed as T).
}

// ============================================================================
// Lists

struct ListNode
{
    u64                                     Value;
    Prism::V3::IntrusiveListHook<ListNode> Hook;
    explicit ListNode(u64 v)
        : Value(v)
    {
    }
};
struct StdListNode
{
    u64 Value;
};

static void RegisterLists()
{
    for (size_t n : kSizes)
    {
        Bench::Add("DoublyLinkedList", Name("PushBack", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::DoublyLinkedList<u64> l;
                    for (size_t i = 0; i < Opaque(n); ++i) l.PushBack(i);
                    return u64(l.Size());
                },
                [n] {
                    std::list<u64> l;
                    for (size_t i = 0; i < Opaque(n); ++i) l.push_back(i);
                    return u64(l.size());
                });
        });
        Bench::Add("DoublyLinkedList", Name("Iterate", n), [n](Runner& r) {
            Prism::DoublyLinkedList<u64> pl;
            std::list<u64>               sl;
            for (u64 k : RandomKeys(n))
            {
                pl.PushBack(k);
                sl.push_back(k);
            }
            r.Compare(
                n,
                [&] {
                    u64 sum = 0;
                    for (u64 x : pl) sum += x;
                    return sum;
                },
                [&] {
                    u64 sum = 0;
                    for (u64 x : sl) sum += x;
                    return sum;
                });
        });
        Bench::Add("DoublyLinkedList", Name("PushFrontPopBack", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::DoublyLinkedList<u64> l;
                    u64                          sum = 0;
                    for (size_t i = 0; i < Opaque(n); ++i) l.PushFront(i);
                    while (!l.Empty()) sum += l.PopBackElement();
                    return sum;
                },
                [n] {
                    std::list<u64> l;
                    u64            sum = 0;
                    for (size_t i = 0; i < Opaque(n); ++i) l.push_front(i);
                    while (!l.empty())
                    {
                        sum += l.back();
                        l.pop_back();
                    }
                    return sum;
                });
        });

        // Intrusive list over preallocated nodes vs std::list, which has to
        // allocate a node per element: what an intrusive list buys you.
        Bench::Add("IntrusiveList", Name("LinkUnlink", n), [n](Runner& r) {
            std::vector<ListNode> nodes;
            nodes.reserve(n);
            for (size_t i = 0; i < n; ++i) nodes.emplace_back(i);
            r.Compare(
                n,
                [&] {
                    Prism::V3::IntrusiveList<ListNode> l;
                    for (auto& node : nodes) l.PushBack(&node);
                    u64 sum = 0;
                    while (ListNode* node = l.TakeFront()) sum += node->Value;
                    return sum;
                },
                [&] {
                    std::list<u64> l;
                    for (auto& node : nodes) l.push_back(node.Value);
                    u64 sum = 0;
                    while (!l.empty())
                    {
                        sum += l.front();
                        l.pop_front();
                    }
                    return sum;
                });
        });
        Bench::Add("IntrusiveList", Name("Iterate", n), [n](Runner& r) {
            std::vector<ListNode> nodes;
            nodes.reserve(n);
            auto keys = RandomKeys(n);
            for (u64 k : keys) nodes.emplace_back(k);
            // Both lists are linked in a shuffled order, so neither walks
            // memory sequentially. std::list allocates its nodes in order;
            // sorting by a random rank relinks them without moving them.
            auto order = UniqueKeys(n);
            Prism::V3::IntrusiveList<ListNode>  pl;
            std::list<std::pair<u64, u64>>      sl;
            for (size_t i = 0; i < n; ++i)
            {
                pl.PushBack(&nodes[order[i] / 2]);
                sl.emplace_back(order[i], nodes[i].Value);
            }
            sl.sort();
            r.Compare(
                n,
                [&] {
                    u64 sum = 0;
                    for (auto& node : pl) sum += node.Value;
                    return sum;
                },
                [&] {
                    u64 sum = 0;
                    for (auto& [rank, value] : sl) sum += value;
                    return sum;
                });
            pl.Clear();
        });
    }
}

// ============================================================================
// Ordered: RedBlackTree vs std::map, Set vs std::set

static void RegisterOrdered()
{
    for (size_t n : kSizes)
    {
        Bench::Add("RedBlackTree", Name("Insert/random", n), [n](Runner& r) {
            auto keys = UniqueKeys(n);
            r.Compare(
                n,
                [&] {
                    Prism::RedBlackTree<u64, u64> t;
                    for (u64 k : keys) t.Insert(k, k * 3);
                    return u64(t.GetSize());
                },
                [&] {
                    std::map<u64, u64> m;
                    for (u64 k : keys) m.emplace(k, k * 3);
                    return u64(m.size());
                });
        });
        Bench::Add("RedBlackTree", Name("Insert/sorted", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    Prism::RedBlackTree<u64, u64> t;
                    for (u64 k = 0; k < Opaque(n); ++k) t.Insert(k, k);
                    return u64(t.GetSize());
                },
                [n] {
                    std::map<u64, u64> m;
                    for (u64 k = 0; k < Opaque(n); ++k) m.emplace_hint(m.end(), k, k);
                    return u64(m.size());
                });
        });
        Bench::Add("RedBlackTree", Name("Find/hit", n), [n](Runner& r) {
            auto                          keys = UniqueKeys(n);
            Prism::RedBlackTree<u64, u64> t;
            std::map<u64, u64>            m;
            for (u64 k : keys)
            {
                t.Insert(k, k * 3);
                m.emplace(k, k * 3);
            }
            auto probes = UniqueKeys(n, 7);
            r.Compare(
                n,
                [&] {
                    u64 sum = 0;
                    for (u64 k : probes) sum += t.At(k);
                    return sum;
                },
                [&] {
                    u64 sum = 0;
                    for (u64 k : probes) sum += m.find(k)->second;
                    return sum;
                });
        });
        Bench::Add("RedBlackTree", Name("Find/miss", n), [n](Runner& r) {
            auto                          keys = UniqueKeys(n);
            Prism::RedBlackTree<u64, u64> t;
            std::map<u64, u64>            m;
            for (u64 k : keys)
            {
                t.Insert(k, k);
                m.emplace(k, k);
            }
            auto probes = UniqueKeys(n, 7);
            for (auto& p : probes) p += 1; // keys are odd, probes even
            r.Compare(
                n,
                [&] {
                    u64 found = 0;
                    for (u64 k : probes) found += t.Contains(k);
                    return found;
                },
                [&] {
                    u64 found = 0;
                    for (u64 k : probes) found += m.contains(k);
                    return found;
                });
        });
        Bench::Add("RedBlackTree", Name("Iterate", n), [n](Runner& r) {
            Prism::RedBlackTree<u64, u64> t;
            std::map<u64, u64>            m;
            for (u64 k : UniqueKeys(n))
            {
                t.Insert(k, k);
                m.emplace(k, k);
            }
            r.Compare(
                n,
                [&] {
                    u64 sum = 0, i = 0;
                    for (auto& kv : t) sum += kv.Key * ++i;
                    return sum;
                },
                [&] {
                    u64 sum = 0, i = 0;
                    for (auto& kv : m) sum += kv.first * ++i;
                    return sum;
                });
        });
        Bench::Add("RedBlackTree", Name("InsertErase", n), [n](Runner& r) {
            auto keys  = UniqueKeys(n);
            auto order = UniqueKeys(n, 3);
            r.Compare(
                n,
                [&] {
                    Prism::RedBlackTree<u64, u64> t;
                    for (u64 k : keys) t.Insert(k, k);
                    u64 erased = 0;
                    for (u64 o : order) erased += t.Erase(o);
                    return erased + t.GetSize();
                },
                [&] {
                    std::map<u64, u64> m;
                    for (u64 k : keys) m.emplace(k, k);
                    u64 erased = 0;
                    for (u64 o : order) erased += m.erase(o);
                    return erased + m.size();
                });
        });

        Bench::Add("Set", Name("Insert/random", n), [n](Runner& r) {
            auto keys = UniqueKeys(n);
            r.Compare(
                n,
                [&] {
                    Prism::Set<u64> s;
                    for (u64 k : keys) s.Insert(k);
                    return u64(s.Size());
                },
                [&] {
                    std::set<u64> s;
                    for (u64 k : keys) s.insert(k);
                    return u64(s.size());
                });
        });
        Bench::Add("Set", Name("Contains/half", n), [n](Runner& r) {
            Prism::Set<u64> ps;
            std::set<u64>   ss;
            for (u64 k : UniqueKeys(n))
            {
                ps.Insert(k);
                ss.insert(k);
            }
            auto probes = RandomKeys(n, 9);
            for (auto& p : probes) p %= 2 * n;
            r.Compare(
                n,
                [&] {
                    u64 found = 0;
                    for (u64 k : probes) found += ps.Contains(k);
                    return found;
                },
                [&] {
                    u64 found = 0;
                    for (u64 k : probes) found += ss.contains(k);
                    return found;
                });
        });
        // Set::LowerBound is not benchmarked: it returns the tree's
        // FindLargestNotAbove (a floor, not a lower bound) and does not
        // convert to Set::Iterator.
    }
}

// ============================================================================
// UnorderedMap vs std::unordered_map

static void RegisterHashMap()
{
    for (size_t n : kSizes)
    {
        Bench::Add("UnorderedMap", Name("Insert/u64", n), [n](Runner& r) {
            auto keys = UniqueKeys(n);
            r.Compare(
                n,
                [&] {
                    Prism::UnorderedMap<u64, u64> m;
                    for (u64 k : keys) m.InsertOrAssign(k, k);
                    return u64(m.Size());
                },
                [&] {
                    std::unordered_map<u64, u64> m;
                    for (u64 k : keys) m.insert_or_assign(k, k);
                    return u64(m.size());
                });
        });
        Bench::Add("UnorderedMap", Name("ReserveInsert/u64", n), [n](Runner& r) {
            auto keys = UniqueKeys(n);
            r.Compare(
                n,
                [&] {
                    Prism::UnorderedMap<u64, u64> m;
                    m.Reserve(n);
                    for (u64 k : keys) m.InsertOrAssign(k, k);
                    return u64(m.Size());
                },
                [&] {
                    std::unordered_map<u64, u64> m;
                    m.reserve(n);
                    for (u64 k : keys) m.insert_or_assign(k, k);
                    return u64(m.size());
                });
        });
        Bench::Add("UnorderedMap", Name("Find/hit/u64", n), [n](Runner& r) {
            Prism::UnorderedMap<u64, u64> pm;
            std::unordered_map<u64, u64>  sm;
            for (u64 k : UniqueKeys(n))
            {
                pm.InsertOrAssign(k, k * 5);
                sm.insert_or_assign(k, k * 5);
            }
            auto probes = UniqueKeys(n, 5);
            r.Compare(
                n,
                [&] {
                    u64 sum = 0;
                    for (u64 k : probes) sum += pm.At(k);
                    return sum;
                },
                [&] {
                    u64 sum = 0;
                    for (u64 k : probes) sum += sm.find(k)->second;
                    return sum;
                });
        });
        Bench::Add("UnorderedMap", Name("Find/miss/u64", n), [n](Runner& r) {
            Prism::UnorderedMap<u64, u64> pm;
            std::unordered_map<u64, u64>  sm;
            for (u64 k : UniqueKeys(n))
            {
                pm.InsertOrAssign(k, k);
                sm.insert_or_assign(k, k);
            }
            auto probes = UniqueKeys(n, 5);
            for (auto& p : probes) p += 1;
            r.Compare(
                n,
                [&] {
                    u64 found = 0;
                    for (u64 k : probes) found += pm.Contains(k);
                    return found;
                },
                [&] {
                    u64 found = 0;
                    for (u64 k : probes) found += sm.contains(k);
                    return found;
                });
        });
        Bench::Add("UnorderedMap", Name("InsertErase/u64", n), [n](Runner& r) {
            auto keys  = UniqueKeys(n);
            auto order = UniqueKeys(n, 13);
            r.Compare(
                n,
                [&] {
                    Prism::UnorderedMap<u64, u64> m;
                    for (u64 k : keys) m.InsertOrAssign(k, k);
                    for (u64 o : order) m.Erase(o);
                    return u64(m.Size());
                },
                [&] {
                    std::unordered_map<u64, u64> m;
                    for (u64 k : keys) m.insert_or_assign(k, k);
                    for (u64 o : order) m.erase(o);
                    return u64(m.size());
                });
        });
        Bench::Add("UnorderedMap", Name("Iterate/u64", n), [n](Runner& r) {
            Prism::UnorderedMap<u64, u64> pm;
            std::unordered_map<u64, u64>  sm;
            for (u64 k : UniqueKeys(n))
            {
                pm.InsertOrAssign(k, k);
                sm.insert_or_assign(k, k);
            }
            r.Compare(
                n,
                [&] {
                    u64 sum = 0;
                    for (auto& kv : pm) sum += kv.Value;
                    return sum;
                },
                [&] {
                    u64 sum = 0;
                    for (auto& kv : sm) sum += kv.second;
                    return sum;
                });
        });
        Bench::Add("UnorderedMap", Name("Insert/string", n), [n](Runner& r) {
            std::vector<std::string>   keys;
            std::vector<Prism::String> pkeys;
            for (u64 k : UniqueKeys(n))
            {
                keys.push_back(KeyString(k));
                pkeys.push_back(Prism::String(keys.back().c_str()));
            }
            r.Compare(
                n,
                [&] {
                    Prism::UnorderedMap<Prism::String, u64> m;
                    for (auto& k : pkeys) m.InsertOrAssign(k, 1);
                    return u64(m.Size());
                },
                [&] {
                    std::unordered_map<std::string, u64> m;
                    for (auto& k : keys) m.insert_or_assign(k, 1);
                    return u64(m.size());
                });
        });
        Bench::Add("UnorderedMap", Name("Find/hit/string", n), [n](Runner& r) {
            std::vector<std::string>   keys;
            std::vector<Prism::String> pkeys;
            for (u64 k : UniqueKeys(n))
            {
                keys.push_back(KeyString(k));
                pkeys.push_back(Prism::String(keys.back().c_str()));
            }
            Prism::UnorderedMap<Prism::String, u64> pm;
            std::unordered_map<std::string, u64>    sm;
            for (size_t i = 0; i < n; ++i)
            {
                pm.InsertOrAssign(pkeys[i], i);
                sm.insert_or_assign(keys[i], i);
            }
            auto order = UniqueKeys(n, 17);
            r.Compare(
                n,
                [&] {
                    u64 sum = 0;
                    for (u64 o : order) sum += pm.At(pkeys[o / 2]);
                    return sum;
                },
                [&] {
                    u64 sum = 0;
                    for (u64 o : order) sum += sm.find(keys[o / 2])->second;
                    return sum;
                });
        });
    }
}

// ============================================================================
// Bitmap vs std::vector<bool>

static void RegisterBitmap()
{
    for (size_t bits : {4096, 1 << 20})
    {
        Bench::Add("Bitmap", Name("SetIndex/random", bits), [bits](Runner& r) {
            Prism::Bitmap     pb;
            pb.Allocate(bits);
            std::vector<bool> sb(bits);
            auto              idx = RandomKeys(4096, 21);
            for (auto& i : idx) i %= bits;
            r.Compare(
                idx.size(),
                [&] {
                    for (u64 i : idx) pb.SetIndex(i, !pb.At(i));
                    return u64(pb.At(idx[0]));
                },
                [&] {
                    for (u64 i : idx) sb[i] = !sb[i];
                    return u64(sb[idx[0]]);
                });
        });
        // Find the first clear bit in a bitmap that is all ones except near
        // the end: a full scan.
        Bench::Add("Bitmap", Name("FindFirstNotSet/scan", bits), [bits](Runner& r) {
            Prism::Bitmap pb;
            pb.Allocate(bits);
            std::vector<bool> sb(bits, true);
            for (size_t i = 0; i < bits; ++i) pb.SetIndex(i, true);
            size_t hole = bits - 3;
            pb.SetIndex(hole, false);
            sb[hole] = false;
            r.Compare(
                1,
                [&] { return u64(pb.FindFirstNotSet()); },
                [&] {
                    auto it = std::find(sb.begin(), sb.end(), false);
                    return u64(it - sb.begin());
                })
                ;
            r.Bytes(double(bits) / 8);
        });
    }
}

int main(int argc, char** argv)
{
    RegisterVector();
    RegisterQueues();
    RegisterLists();
    RegisterOrdered();
    RegisterHashMap();
    RegisterBitmap();
    return Bench::Main(argc, argv, "Containers");
}
