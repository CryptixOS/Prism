/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

// Prism algorithms, hashing, random numbers, vocabulary types, atomics,
// locks, paths, math and ranges vs their std counterparts. See
// Common/Bench.hpp for the flags. (Sorting has its own: Benchmarks/Sort.)

#include "../Common/Bench.hpp"

#include <algorithm>
#include <atomic>
#include <expected>
#include <filesystem>
#include <functional>
#include <mutex>
#include <optional>
#include <random>
#include <ranges>
#include <vector>

import Prism.Algorithm.Find;
import Prism.Algorithm.Hash;
import Prism.Algorithm.Random;
import Prism.Containers.Vector;
import Prism.Math.Matrix;
import Prism.Math.Vector;
import Prism.Ranges.Views;
import Prism.Utility.Atomic;
import Prism.Utility.Checksum;
import Prism.Utility.Delegate;
import Prism.Utility.Expected;
import Prism.Utility.Mutex;
import Prism.Utility.Optional;
import Prism.Utility.Path;

using Bench::Name;
using Bench::Opaque;
using Bench::Runner;
using Bench::u64;

static std::vector<u64> RandomValues(size_t n, u64 seed, u64 bound = ~u64(0))
{
    Bench::Rng       rng(seed);
    std::vector<u64> v(n);
    for (auto& x : v) x = bound == ~u64(0) ? rng.Next() : rng.Below(bound);
    return v;
}

// ============================================================================
// Find / Count / AnyOf vs <algorithm>

static void RegisterFind()
{
    for (size_t n : {64, 4096, 262144})
    {
        // The value sits at the end: a full scan.
        Bench::Add("Find", Name("Find/u32", n), [n](Runner& r) {
            std::vector<std::uint32_t> v(n, 1);
            v.back() = 2;
            r.Compare(
                n, [&] { return u64(Prism::Find(v.begin(), v.end(), Opaque(2u)) - v.begin()); },
                [&] { return u64(std::find(v.begin(), v.end(), Opaque(2u)) - v.begin()); });
        });
        Bench::Add("Find", Name("FindIf/u32", n), [n](Runner& r) {
            std::vector<std::uint32_t> v(n, 1);
            v.back() = 9;
            auto pred = [](std::uint32_t x) { return x > 5; };
            r.Compare(
                n, [&] { return u64(Prism::FindIf(v.begin(), v.end(), pred) - v.begin()); },
                [&] { return u64(std::find_if(v.begin(), v.end(), pred) - v.begin()); });
        });
        Bench::Add("Find", Name("Count/u8", n), [n](Runner& r) {
            std::vector<std::uint8_t> v(n);
            Bench::Rng                rng(2);
            for (auto& x : v) x = std::uint8_t(rng.Below(4));
            r.Compare(
                n, [&] { return u64(Prism::Count(v.begin(), v.end(), Opaque(std::uint8_t(3)))); },
                [&] { return u64(std::count(v.begin(), v.end(), Opaque(std::uint8_t(3)))); });
        });
        Bench::Add("Find", Name("CountIf/u64", n), [n](Runner& r) {
            auto v    = RandomValues(n, 3, 100);
            auto pred = [](u64 x) { return x < 30; };
            r.Compare(
                n, [&] { return u64(Prism::CountIf(v.begin(), v.end(), pred)); },
                [&] { return u64(std::count_if(v.begin(), v.end(), pred)); });
        });
        Bench::Add("Find", Name("AllOf/u64", n), [n](Runner& r) {
            auto v    = RandomValues(n, 4, 100);
            auto pred = [](u64 x) { return x < 100; };
            r.Compare(
                n, [&] { return u64(Prism::AllOf(v.begin(), v.end(), pred)); },
                [&] { return u64(std::all_of(v.begin(), v.end(), pred)); });
        });
    }
}

// ============================================================================
// Hashes and checksums: no std counterpart, absolute throughput only
// (std::hash<string_view> is listed next to them for scale).

static void RegisterHash()
{
    for (size_t n : {16, 256, 65536})
    {
        std::vector<std::uint8_t> data(n);
        {
            Bench::Rng rng(5);
            for (auto& b : data) b = std::uint8_t(rng.Next());
        }
        Bench::Add("Hash", Name("FNV1a", n), [n, data](Runner& r) mutable {
            r.Bytes(double(n));
            r.Single(1, [&] { return u64(Prism::FNV1a::Hash(Opaque(data.data()), n)); });
        });
        Bench::Add("Hash", Name("Murmur2", n), [n, data](Runner& r) mutable {
            r.Bytes(double(n));
            r.Single(1, [&] { return u64(Prism::Murmur::Hash2(Opaque(data.data()), n, 0x1234)); });
        });
        Bench::Add("Hash", Name("Murmur3-128", n), [n, data](Runner& r) mutable {
            r.Bytes(double(n));
            r.Single(1, [&] {
                auto h = Prism::Murmur::Hash3(Prism::Span<std::uint8_t>(Opaque(data.data()), n), 0x1234);
                return u64(h.Low() ^ h.High());
            });
        });
        Bench::Add("Hash", Name("std::hash<string_view>", n), [n, data](Runner& r) {
            std::string_view view(reinterpret_cast<const char*>(data.data()), n);
            r.Bytes(double(n));
            r.Single(1, [&] { return u64(std::hash<std::string_view>{}(Opaque(view))); });
        });
        Bench::Add("Checksum", Name("CRC32", n), [n, data](Runner& r) {
            r.Bytes(double(n));
            r.Single(1, [&] { return u64(Prism::CRC32::DoChecksum(Opaque(data.data()), n)); });
        });
        Bench::Add("Checksum", Name("Adler32", n), [n, data](Runner& r) {
            r.Bytes(double(n));
            r.Single(1, [&] { return u64(Prism::Adler32::DoChecksum(Opaque(data.data()), n)); });
        });
    }
}

// ============================================================================
// Random vs <random>

static void RegisterRandom()
{
    constexpr size_t n = 4096;
    // Same engine parameters and seed: the streams must match exactly.
    Bench::Add("Random", "mt19937", [](Runner& r) {
        Prism::Algorithm::mt19937 pg(12345);
        std::mt19937   sg(12345);
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i) sum = sum * 31 + pg();
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i) sum = sum * 31 + sg();
                return sum;
            });
    });
    Bench::Add("Random", "mt19937_64", [](Runner& r) {
        Prism::Algorithm::mt19937_64 pg(12345);
        std::mt19937_64   sg(12345);
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i) sum = sum * 31 + pg();
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i) sum = sum * 31 + sg();
                return sum;
            });
    });
    Bench::Add("Random", "mt19937/seed", [](Runner& r) {
        r.Compare(
            1,
            [] {
                Prism::Algorithm::mt19937 g(Opaque(7u));
                return u64(g());
            },
            [] {
                std::mt19937 g(Opaque(7u));
                return u64(g());
            });
    });
    // The two distributions may map the same engine output differently, so
    // the values are not compared.
    Bench::Add("Random", "UniformInt/0..999", [](Runner& r) {
        Prism::Algorithm::mt19937 pg(99);
        std::mt19937   sg(99);
        r.Compare(
            n,
            [&] {
                Prism::Algorithm::UniformIntDistribution<int> dist(0, 999);
                u64                                sum = 0;
                for (size_t i = 0; i < n; ++i) sum += dist(pg);
                return sum;
            },
            [&] {
                std::uniform_int_distribution<int> dist(0, 999);
                u64                                sum = 0;
                for (size_t i = 0; i < n; ++i) sum += dist(sg);
                return sum;
            },
            Bench::NoCheck);
    });
}

// ============================================================================
// Optional / Expected / Delegate

static Prism::Optional<u64> PrismMaybe(u64 x)
{
    if (x % 3 == 0) return {};
    return x * 2;
}
static std::optional<u64> StdMaybe(u64 x)
{
    if (x % 3 == 0) return std::nullopt;
    return x * 2;
}
[[gnu::noinline]] static Prism::Expected<u64, int> PrismParse(u64 x)
{
    if (x % 5 == 0) return Prism::Unexpected<int>(int(x % 7));
    return x + 1;
}
[[gnu::noinline]] static std::expected<u64, int> StdParse(u64 x)
{
    if (x % 5 == 0) return std::unexpected<int>(int(x % 7));
    return x + 1;
}

static int AddOne(int x) { return x + 1; }

static void RegisterVocabulary()
{
    constexpr size_t n = 4096;
    auto             inputs = RandomValues(n, 6, 1000);

    Bench::Add("Optional", "ReturnAndTest", [inputs](Runner& r) {
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (u64 x : inputs)
                    if (auto o = PrismMaybe(x); o.HasValue()) sum += o.Value();
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (u64 x : inputs)
                    if (auto o = StdMaybe(x); o.has_value()) sum += *o;
                return sum;
            });
    });
    Bench::Add("Optional", "ValueOr", [inputs](Runner& r) {
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (u64 x : inputs) sum += PrismMaybe(x).ValueOr(1);
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (u64 x : inputs) sum += StdMaybe(x).value_or(1);
                return sum;
            });
    });

    Bench::Add("Expected", "ReturnAndTest", [inputs](Runner& r) {
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (u64 x : inputs)
                {
                    auto e = PrismParse(x);
                    sum += e.HasValue() ? e.Value() : u64(e.Error());
                }
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (u64 x : inputs)
                {
                    auto e = StdParse(x);
                    sum += e.has_value() ? e.value() : u64(e.error());
                }
                return sum;
            });
    });

    // Prism::Variant is not benchmarked: Variant.ixx is left out of the
    // build (it does not compile yet).

    Bench::Add("Delegate", "Invoke/lambda", [](Runner& r) {
        int                     bias = Opaque(3);
        Prism::Delegate<int(int)> pd([bias](int x) { return x + bias; });
        std::function<int(int)>   sf([bias](int x) { return x + bias; });
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i) sum += Opaque(&pd)[0](int(i));
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i) sum += Opaque(&sf)[0](int(i));
                return sum;
            });
    });
    Bench::Add("Delegate", "Invoke/function-pointer", [](Runner& r) {
        Prism::Delegate<int(int)> pd;
        pd.Bind<&AddOne>();
        std::function<int(int)> sf(&AddOne);
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i) sum += Opaque(&pd)[0](int(i));
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i) sum += Opaque(&sf)[0](int(i));
                return sum;
            });
    });
    Bench::Add("Delegate", "Construct/small-lambda", [](Runner& r) {
        r.Compare(
            n,
            [] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i)
                {
                    Prism::Delegate<int(int)> d([i](int x) { return x + int(i); });
                    sum += Opaque(&d)[0](1);
                }
                return sum;
            },
            [] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i)
                {
                    std::function<int(int)> f([i](int x) { return x + int(i); });
                    sum += Opaque(&f)[0](1);
                }
                return sum;
            });
    });
    // 48 bytes of captures: past std::function's small buffer.
    Bench::Add("Delegate", "Construct/large-lambda", [](Runner& r) {
        r.Compare(
            n,
            [] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i)
                {
                    u64 a = i, b = 1, c = 2, d = 3, e = 4, f = 5;
                    Prism::Delegate<int(int)> del(
                        [a, b, c, d, e, f](int x) { return x + int(a + b + c + d + e + f); });
                    sum += Opaque(&del)[0](1);
                }
                return sum;
            },
            [] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i)
                {
                    u64 a = i, b = 1, c = 2, d = 3, e = 4, f = 5;
                    std::function<int(int)> fn(
                        [a, b, c, d, e, f](int x) { return x + int(a + b + c + d + e + f); });
                    sum += Opaque(&fn)[0](1);
                }
                return sum;
            });
    });
}

// ============================================================================
// Atomic / Spinlock / Mutex (uncontended, single thread)

static void RegisterSync()
{
    constexpr size_t n = 4096;
    Bench::Add("Atomic", "FetchAdd/seq_cst", [](Runner& r) {
        Prism::Atomic<u64> pa{0};
        std::atomic<u64>   sa{0};
        r.Compare(
            n,
            [&] {
                pa.Store(0);
                for (size_t i = 0; i < n; ++i) pa.FetchAdd(i);
                return pa.Load();
            },
            [&] {
                sa.store(0);
                for (size_t i = 0; i < n; ++i) sa.fetch_add(i);
                return sa.load();
            });
    });
    Bench::Add("Atomic", "Load/acquire", [](Runner& r) {
        Prism::Atomic<u64> pa{5};
        std::atomic<u64>   sa{5};
        r.Compare(
            n,
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i) sum += pa.Load(Prism::MemoryOrder::eAcquire);
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (size_t i = 0; i < n; ++i) sum += sa.load(std::memory_order_acquire);
                return sum;
            });
    });
    Bench::Add("Atomic", "CompareExchange", [](Runner& r) {
        Prism::Atomic<u64> pa{0};
        std::atomic<u64>   sa{0};
        r.Compare(
            n,
            [&] {
                pa.Store(0);
                for (size_t i = 0; i < n; ++i)
                {
                    u64 expected = i;
                    pa.CompareExchange(expected, i + 1, false, Prism::MemoryOrder::eSeqCst,
                                       Prism::MemoryOrder::eSeqCst);
                }
                return pa.Load();
            },
            [&] {
                sa.store(0);
                for (size_t i = 0; i < n; ++i)
                {
                    u64 expected = i;
                    sa.compare_exchange_strong(expected, i + 1);
                }
                return sa.load();
            });
    });
    Bench::Add("Lock", "Spinlock-vs-std::mutex", [](Runner& r) {
        Prism::Spinlock lock;
        std::mutex      mutex;
        u64             pc = 0, sc = 0;
        r.Compare(
            n,
            [&] {
                for (size_t i = 0; i < n; ++i)
                {
                    lock.Lock();
                    ++Opaque(&pc)[0];
                    lock.Unlock();
                }
                return pc % n;
            },
            [&] {
                for (size_t i = 0; i < n; ++i)
                {
                    mutex.lock();
                    ++Opaque(&sc)[0];
                    mutex.unlock();
                }
                return sc % n;
            });
    });
    Bench::Add("Lock", "Mutex-vs-std::mutex", [](Runner& r) {
        Prism::Mutex lock;
        std::mutex   mutex;
        u64          pc = 0, sc = 0;
        r.Compare(
            n,
            [&] {
                for (size_t i = 0; i < n; ++i)
                {
                    lock.Lock();
                    ++Opaque(&pc)[0];
                    lock.Unlock();
                }
                return pc % n;
            },
            [&] {
                for (size_t i = 0; i < n; ++i)
                {
                    mutex.lock();
                    ++Opaque(&sc)[0];
                    mutex.unlock();
                }
                return sc % n;
            });
    });
}

// ============================================================================
// Path vs std::filesystem::path

static void RegisterPath()
{
    const char* paths[] = {
        "/usr/lib/x86_64-linux-gnu/libprism.so.1",
        "/home/user/projects/prism/Source/Prism/Utility/Path.ixx",
        "relative/dir/file.tar.gz",
        "/boot/kernel",
    };
    Bench::Add("Path", "Construct", [paths](Runner& r) {
        r.Compare(
            4,
            [&] {
                u64 sum = 0;
                for (auto* p : paths) sum += Prism::Path(Opaque(p)).Size();
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (auto* p : paths) sum += std::filesystem::path(Opaque(p)).native().size();
                return sum;
            });
    });
    // Different conventions ("tar.gz" vs ".gz"...) so only the speed is
    // compared.
    Bench::Add("Path", "BaseName+Extension+Parent", [paths](Runner& r) {
        std::vector<Prism::Path>           pp;
        std::vector<std::filesystem::path> sp;
        for (auto* p : paths)
        {
            pp.emplace_back(p);
            sp.emplace_back(p);
        }
        r.Compare(
            4,
            [&] {
                u64 sum = 0;
                for (auto& p : pp)
                    sum += p.BaseName().Size() + p.Extension().Size() + p.ParentPath().Size();
                return sum;
            },
            [&] {
                u64 sum = 0;
                for (auto& p : sp)
                    sum += p.filename().native().size() + p.extension().native().size()
                         + p.parent_path().native().size();
                return sum;
            },
            Bench::NoCheck);
    });
}

// ============================================================================
// Math: Matrix / Vector against plain loops on arrays

static void RegisterMath()
{
    using M4 = Prism::Math::Matrix<4, 4, float>;
    using V3 = Prism::Math::Vector<3, float>;
    constexpr size_t n = 1024;

    Bench::Add("Math", "Matrix4x4-multiply", [](Runner& r) {
        std::vector<M4>                                   pm(n);
        std::vector<std::array<std::array<float, 4>, 4>> am(n);
        Bench::Rng                                        rng(8);
        for (size_t k = 0; k < n; ++k)
            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                {
                    float v  = float(rng.Below(100)) / 10.0f;
                    pm[k][i][j] = v;
                    am[k][i][j] = v;
                }
        r.Compare(
            n - 1,
            [&] {
                float sum = 0;
                for (size_t k = 0; k + 1 < n; ++k)
                {
                    M4 c = pm[k] * pm[k + 1];
                    sum += c[0][0] + c[3][3];
                }
                return u64(sum);
            },
            [&] {
                float sum = 0;
                for (size_t k = 0; k + 1 < n; ++k)
                {
                    // Column-major like Prism's: m[col][row].
                    std::array<std::array<float, 4>, 4> c{};
                    for (int col = 0; col < 4; ++col)
                        for (int row = 0; row < 4; ++row)
                        {
                            float acc = 0;
                            for (int t = 0; t < 4; ++t)
                                acc += am[k][t][row] * am[k + 1][col][t];
                            c[col][row] = acc;
                        }
                    sum += c[0][0] + c[3][3];
                }
                return u64(sum);
            });
    });
    Bench::Add("Math", "Vector3-dot+cross+normalize", [](Runner& r) {
        std::vector<V3>                   pv(n);
        std::vector<std::array<float, 3>> av(n);
        Bench::Rng                        rng(9);
        for (size_t k = 0; k < n; ++k)
            for (int i = 0; i < 3; ++i)
            {
                float v  = 1.0f + float(rng.Below(100)) / 10.0f;
                pv[k][i] = v;
                av[k][i] = v;
            }
        r.Compare(
            n - 1,
            [&] {
                float sum = 0;
                for (size_t k = 0; k + 1 < n; ++k)
                {
                    V3 c = pv[k].Cross(pv[k + 1]).Normalized();
                    sum += c.Dot(pv[k]);
                }
                return u64(sum * 1000);
            },
            [&] {
                float sum = 0;
                for (size_t k = 0; k + 1 < n; ++k)
                {
                    auto& a = av[k];
                    auto& b = av[k + 1];
                    float c[3] = {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2],
                                  a[0] * b[1] - a[1] * b[0]};
                    float len  = std::sqrt(c[0] * c[0] + c[1] * c[1] + c[2] * c[2]);
                    for (float& x : c) x /= len;
                    sum += c[0] * a[0] + c[1] * a[1] + c[2] * a[2];
                }
                return u64(sum * 1000);
            });
    });
}

// ============================================================================
// Ranges: Views::Filter | Transform vs std::views

static void RegisterRanges()
{
    for (size_t n : {64, 65536})
    {
        Bench::Add("Ranges", Name("Filter|Transform-sum", n), [n](Runner& r) {
            auto                     values = RandomValues(n, 10, 1000);
            Prism::Vector<u64>       pv(values.begin(), values.end());
            r.Compare(
                n,
                [&] {
                    u64 sum = 0;
                    for (u64 x : pv | Prism::Views::Filter([](u64 v) { return v % 2 == 0; })
                                     | Prism::Views::Transform([](u64 v) { return v * v; }))
                        sum += x;
                    return sum;
                },
                [&] {
                    u64 sum = 0;
                    for (u64 x : values | std::views::filter([](u64 v) { return v % 2 == 0; })
                                     | std::views::transform([](u64 v) { return v * v; }))
                        sum += x;
                    return sum;
                });
        });
        Bench::Add("Ranges", Name("Iota|Take-sum", n), [n](Runner& r) {
            r.Compare(
                n,
                [n] {
                    u64 sum = 0;
                    for (u64 x : Prism::Views::Iota(u64(0)) | Prism::Views::Take(Opaque(n)))
                        sum += x * 3;
                    return sum;
                },
                [n] {
                    u64 sum = 0;
                    for (u64 x : std::views::iota(u64(0)) | std::views::take(Opaque(n)))
                        sum += x * 3;
                    return sum;
                });
        });
    }
}

int main(int argc, char** argv)
{
    RegisterFind();
    RegisterHash();
    RegisterRandom();
    RegisterVocabulary();
    RegisterSync();
    RegisterPath();
    RegisterMath();
    RegisterRanges();
    return Bench::Main(argc, argv, "Utility");
}
