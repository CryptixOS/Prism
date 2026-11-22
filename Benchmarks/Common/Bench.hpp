/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

// A small micro-benchmark harness shared by the Benchmarks/* executables.
// Only std headers, no imports (headers never import, see the modules
// layout), so include it before the import declarations.
//
// A case is registered lazily: its body runs (and does its setup) only when
// the case is selected. The body calls exactly one of
//
//   r.Compare(ops, prism, std)   Prism vs the std equivalent
//   r.Single(ops, prism)         no std counterpart: absolute numbers only
//
// Each callable does one batch of `ops` operations and returns a u64
// checksum. The two sides of a Compare must return the same checksum
// (pass Bench::NoCheck when they legitimately differ, e.g. two different
// hash functions), which catches a benchmark that measures the wrong thing
// as well as a Prism bug. The sides run alternately to cancel out drift and
// the median of the samples is reported in ns/op; ratio = prism / std, so
// < 1 means Prism is faster.
//
//   <Bench> [--filter substr[,substr]] [--min-time-ms 20] [--reps 9]
//           [--csv out.csv] [--list] [--quick]
//
// Build with CMAKE_BUILD_TYPE=Release and, for stable numbers, pin it:
// taskset -c 2 ./Containers

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace Bench
{
    using u64   = std::uint64_t;
    using i64   = std::int64_t;
    using Clock = std::chrono::steady_clock;

    template <typename T>
    inline void DoNotOptimize(const T& value)
    {
        if constexpr (std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(void*))
            asm volatile("" : : "r,m"(value) : "memory");
        else asm volatile("" : : "m"(value) : "memory");
    }
    // A register constraint only for values that fit in one: GCC will
    // otherwise pick "r" for a 16-byte struct and clobber half of it. And a
    // single alternative: GCC 16 miscompiles "+m,r" (a captured local read
    // as uninitialized on the next call).
    template <typename T>
    inline void DoNotOptimize(T& value)
    {
        if constexpr (std::is_trivially_copyable_v<T> && sizeof(T) <= sizeof(void*))
            asm volatile("" : "+r"(value) : : "memory");
        else asm volatile("" : "+m"(value) : : "memory");
    }
    inline void ClobberMemory() { asm volatile("" : : : "memory"); }

    // Hides a value from the optimizer so loop bounds and inputs are not
    // constant-folded.
    template <typename T>
    inline T Opaque(T value)
    {
        DoNotOptimize(value);
        return value;
    }

    // splitmix64, the same sequence on every run.
    struct Rng
    {
        u64 State;
        explicit Rng(u64 seed = 0x5eed) : State(seed) {}
        u64 Next()
        {
            u64 z = (State += 0x9e3779b97f4a7c15ull);
            z     = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ull;
            z     = (z ^ (z >> 27)) * 0x94d049bb133111ebull;
            return z ^ (z >> 31);
        }
        u64 Below(u64 bound) { return Next() % bound; }
    };

    inline constexpr struct NoCheckTag
    {
    } NoCheck{};

    struct Options
    {
        double                   MinTimeMs = 20.0;
        int                      Reps      = 9;
        bool                     List      = false;
        bool                     Quick     = false;
        std::vector<std::string> Filters;
        std::string              Csv;
    };

    struct Result
    {
        std::string Group, Name;
        double      PrismNs = 0, StdNs = 0;
        bool        HasStd   = false;
        bool        Mismatch = false;
        u64         PrismSum = 0, StdSum = 0;
        double      Bytes    = 0;
    };

    class Runner;
    struct Case
    {
        std::string                  Group, Name;
        std::function<void(Runner&)> Body;
    };

    inline std::vector<Case>& Registry()
    {
        static std::vector<Case> cases;
        return cases;
    }
    inline Options& Opts()
    {
        static Options options;
        return options;
    }

    inline void Add(std::string group, std::string name,
                    std::function<void(Runner&)> body)
    {
        Registry().push_back({std::move(group), std::move(name), std::move(body)});
    }

    // Formats a case name: Name("Find", "hit", n) -> "Find/hit/1000".
    template <typename... Parts>
    inline std::string Name(const Parts&... parts)
    {
        std::string out;
        auto        append = [&](const auto& part)
        {
            if (!out.empty()) out += '/';
            if constexpr (std::is_arithmetic_v<std::decay_t<decltype(part)>>)
                out += std::to_string(part);
            else out += part;
        };
        (append(parts), ...);
        return out;
    }

    class Runner
    {
      public:
        Result Current;
        bool   Ran = false;

        // Bytes processed per op: also print GB/s.
        Runner& Bytes(double bytesPerOp)
        {
            Current.Bytes = bytesPerOp;
            return *this;
        }

        template <typename P, typename S>
        void Compare(u64 ops, P&& prism, S&& std)
        {
            CompareImpl(ops, prism, std, true);
        }
        template <typename P, typename S>
        void Compare(u64 ops, P&& prism, S&& std, NoCheckTag)
        {
            CompareImpl(ops, prism, std, false);
        }
        template <typename P>
        void Single(u64 ops, P&& prism)
        {
            Ran = true;
            DoNotOptimize(prism());
            u64 batches = Calibrate(prism);
            std::vector<double> samples;
            for (int rep = 0; rep < Opts().Reps; ++rep)
                samples.push_back(Sample(prism, batches) / double(ops));
            Current.PrismNs = Median(samples);
        }

      private:
        template <typename F>
        static double Sample(F& fn, u64 batches)
        {
            auto start = Clock::now();
            for (u64 i = 0; i < batches; ++i)
            {
                u64 result = fn();
                DoNotOptimize(result);
            }
            auto end = Clock::now();
            return std::chrono::duration<double, std::nano>(end - start).count()
                 / double(batches);
        }
        // Batches per sample so each sample takes MinTime / Reps.
        template <typename F>
        static u64 Calibrate(F& fn)
        {
            double target = Opts().MinTimeMs * 1e6 / Opts().Reps;
            u64    batches = 1;
            for (;;)
            {
                double ns = Sample(fn, batches) * double(batches);
                if (ns >= target / 4 || batches >= (u64(1) << 30))
                {
                    double per = ns / double(batches);
                    return std::max<u64>(1, u64(target / std::max(per, 1.0)));
                }
                batches *= 4;
            }
        }
        static double Median(std::vector<double>& v)
        {
            std::sort(v.begin(), v.end());
            return v[v.size() / 2];
        }

        template <typename P, typename S>
        void CompareImpl(u64 ops, P& prism, S& std, bool check)
        {
            Ran            = true;
            Current.HasStd = true;
            // Warm up, and check both sides compute the same thing.
            u64 a = prism(), b = std();
            if (check && a != b)
            {
                Current.Mismatch = true;
                Current.PrismSum = a;
                Current.StdSum   = b;
            }

            u64 batches = std::min(Calibrate(prism), Calibrate(std));
            std::vector<double> ps, ss;
            for (int rep = 0; rep < Opts().Reps; ++rep)
            {
                if (rep & 1)
                {
                    ss.push_back(Sample(std, batches) / double(ops));
                    ps.push_back(Sample(prism, batches) / double(ops));
                }
                else
                {
                    ps.push_back(Sample(prism, batches) / double(ops));
                    ss.push_back(Sample(std, batches) / double(ops));
                }
            }
            Current.PrismNs = Median(ps);
            Current.StdNs   = Median(ss);
        }
    };

    inline bool Selected(const Case& c)
    {
        if (Opts().Filters.empty()) return true;
        std::string full = c.Group + "/" + c.Name;
        for (auto& f : Opts().Filters)
            if (full.find(f) != std::string::npos) return true;
        return false;
    }

    inline std::string FormatNs(double ns)
    {
        char buffer[32];
        if (ns < 10) std::snprintf(buffer, sizeof(buffer), "%.3f", ns);
        else if (ns < 1000) std::snprintf(buffer, sizeof(buffer), "%.1f", ns);
        else std::snprintf(buffer, sizeof(buffer), "%.0f", ns);
        return buffer;
    }

    inline void Usage(const char* argv0)
    {
        std::printf("usage: %s [--filter a,b] [--min-time-ms N] [--reps N] "
                    "[--csv FILE] [--list] [--quick]\n",
                    argv0);
    }

    inline int Main(int argc, char** argv, const char* title)
    {
        Options& o = Opts();
        for (int i = 1; i < argc; ++i)
        {
            std::string_view arg = argv[i];
            auto             next = [&]() -> const char*
            {
                if (i + 1 >= argc)
                {
                    Usage(argv[0]);
                    std::exit(2);
                }
                return argv[++i];
            };
            if (arg == "--filter")
            {
                std::string list = next();
                for (size_t start = 0; start <= list.size();)
                {
                    size_t comma = list.find(',', start);
                    if (comma == std::string::npos) comma = list.size();
                    if (comma > start)
                        o.Filters.push_back(list.substr(start, comma - start));
                    start = comma + 1;
                }
            }
            else if (arg == "--min-time-ms") o.MinTimeMs = std::atof(next());
            else if (arg == "--reps") o.Reps = std::max(1, std::atoi(next()));
            else if (arg == "--csv") o.Csv = next();
            else if (arg == "--list") o.List = true;
            else if (arg == "--quick") o.Quick = true;
            else
            {
                Usage(argv[0]);
                return arg == "--help" || arg == "-h" ? 0 : 2;
            }
        }
        if (o.Quick)
        {
            o.MinTimeMs = std::min(o.MinTimeMs, 5.0);
            o.Reps      = std::min(o.Reps, 5);
        }

        // Cases are registered size by size; keep each group together, in
        // order of first appearance.
        {
            std::map<std::string, size_t> rank;
            for (auto& c : Registry()) rank.emplace(c.Group, rank.size());
            std::stable_sort(Registry().begin(), Registry().end(),
                             [&](const Case& a, const Case& b)
                             { return rank[a.Group] < rank[b.Group]; });
        }

        if (o.List)
        {
            for (auto& c : Registry())
                if (Selected(c)) std::printf("%s/%s\n", c.Group.c_str(), c.Name.c_str());
            return 0;
        }

        std::printf("%s: prism vs std, ns/op (median of %d), ratio = prism/std, "
                    "< 1 is faster\n",
                    title, o.Reps);
        std::vector<Result> results;
        std::string         group;
        for (auto& c : Registry())
        {
            if (!Selected(c)) continue;
            if (c.Group != group)
            {
                group = c.Group;
                std::printf("\n== %s\n%-44s %12s %12s %8s\n", group.c_str(),
                            "case", "prism", "std", "ratio");
            }
            Runner r;
            r.Current.Group = c.Group;
            r.Current.Name  = c.Name;
            c.Body(r);
            if (!r.Ran) continue;
            Result& res = r.Current;
            std::printf("%-44s %12s", res.Name.c_str(), FormatNs(res.PrismNs).c_str());
            if (res.HasStd)
            {
                double ratio = res.PrismNs / res.StdNs;
                const char* mark = res.Mismatch ? "  MISMATCH"
                                 : ratio > 1.10 ? "  slower"
                                 : ratio < 0.90 ? "  faster"
                                                : "";
                std::printf(" %12s %8.3f%s", FormatNs(res.StdNs).c_str(), ratio, mark);
            }
            else std::printf(" %12s %8s", "-", "-");
            if (res.Bytes > 0)
                std::printf("  (%.2f GB/s)", res.Bytes / res.PrismNs);
            if (res.Mismatch)
                std::printf("  [prism %llu, std %llu]",
                            static_cast<unsigned long long>(res.PrismSum),
                            static_cast<unsigned long long>(res.StdSum));
            std::printf("\n");
            std::fflush(stdout);
            results.push_back(res);
        }

        // Geometric mean of the ratios, per group and overall.
        struct Acc
        {
            double LogSum = 0;
            int    N = 0, Faster = 0, Slower = 0, Mismatch = 0;
        };
        std::vector<std::string> order;
        std::map<std::string, Acc> groups;
        Acc                        total;
        for (auto& res : results)
        {
            if (!res.HasStd) continue;
            if (!groups.count(res.Group)) order.push_back(res.Group);
            double ratio = res.PrismNs / res.StdNs;
            for (Acc* acc : {&groups[res.Group], &total})
            {
                acc->LogSum += std::log(ratio);
                acc->N++;
                acc->Faster += ratio < 0.90;
                acc->Slower += ratio > 1.10;
                acc->Mismatch += res.Mismatch;
            }
        }
        if (total.N)
        {
            std::printf("\n== Summary (geomean of prism/std; faster/slower = "
                        "beyond 10%%)\n%-28s %8s %6s %7s %7s\n",
                        "group", "geomean", "cases", "faster", "slower");
            auto line = [](const std::string& name, const Acc& acc)
            {
                std::printf("%-28s %8.3f %6d %7d %7d%s\n", name.c_str(),
                            std::exp(acc.LogSum / acc.N), acc.N, acc.Faster,
                            acc.Slower,
                            acc.Mismatch ? "  MISMATCH" : "");
            };
            for (auto& name : order) line(name, groups[name]);
            line("overall", total);
        }

        if (!o.Csv.empty())
        {
            std::FILE* f = std::fopen(o.Csv.c_str(), "w");
            if (!f)
            {
                std::perror(o.Csv.c_str());
                return 1;
            }
            std::fprintf(f, "group,case,prism_ns,std_ns,ratio,mismatch\n");
            for (auto& res : results)
                std::fprintf(f, "%s,%s,%.4f,%.4f,%.4f,%d\n", res.Group.c_str(),
                             res.Name.c_str(), res.PrismNs,
                             res.HasStd ? res.StdNs : 0.0,
                             res.HasStd ? res.PrismNs / res.StdNs : 0.0,
                             int(res.Mismatch));
            std::fclose(f);
        }
        return total.Mismatch ? 1 : 0;
    }
} // namespace Bench
