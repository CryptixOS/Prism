/*
 * Created by v1tr10l7 on 13.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

// Prism::Sort vs std::sort.
//
// Every case is (size, pattern, element type + comparator). Both sorts get
// the same input, are run alternately to cancel out drift, and the median
// of the repetitions is reported in ns per element. ratio = prism / std,
// so < 1 means Prism is faster. Each case is also checked: both outputs
// must be sorted, agree element by element (up to equivalence) and keep
// every payload. A separate counting run reports comparisons per element.
//
//   Sort [--quick] [--sizes 1000,100000] [--patterns random,sorted]
//        [--types i32,string] [--min-time-ms 30] [--csv out.csv] [--list]
//
// Build with optimizations (CMAKE_BUILD_TYPE=Release) and, for stable
// numbers, pin it: taskset -c 2 ./Sort

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

import Prism.Algorithm.Sort;

using u64   = std::uint64_t;
using i64   = std::int64_t;
using Clock = std::chrono::steady_clock;

// ============================================================================
// Element types

struct Pair16
{
    u64         Key;
    u64         Payload;
    friend bool operator<(const Pair16& a, const Pair16& b)
    {
        return a.Key < b.Key;
    }
    friend bool operator>(const Pair16& a, const Pair16& b)
    {
        return a.Key > b.Key;
    }
};

struct Big64
{
    u64         Key;
    u64         Payload[7];
    friend bool operator<(const Big64& a, const Big64& b)
    {
        return a.Key < b.Key;
    }
    friend bool operator>(const Big64& a, const Big64& b)
    {
        return a.Key > b.Key;
    }
};

// Keys share a long prefix so every comparison walks a few bytes.
static std::string MakeString(u64 key)
{
    char buffer[48];
    std::snprintf(buffer, sizeof(buffer), "prism/bench/key-%012llu",
                  static_cast<unsigned long long>(key));
    return buffer;
}

// Order-independent fingerprint of the payloads, to catch a sort that
// duplicates or drops elements whose keys compare equal.
static u64 Fingerprint(std::uint8_t v) { return v * 0x9e3779b97f4a7c15ull; }
static u64 Fingerprint(int v)
{
    return static_cast<u64>(static_cast<std::uint32_t>(v))
         * 0x9e3779b97f4a7c15ull;
}
static u64 Fingerprint(i64 v)
{
    return static_cast<u64>(v) * 0x9e3779b97f4a7c15ull;
}
static u64 Fingerprint(double v)
{
    u64 bits;
    std::memcpy(&bits, &v, sizeof(bits));
    return bits * 0x9e3779b97f4a7c15ull;
}
static u64 Fingerprint(const Pair16& v)
{
    return (v.Key ^ (v.Payload << 21)) * 0x9e3779b97f4a7c15ull;
}
static u64 Fingerprint(const Big64& v)
{
    return (v.Key ^ (v.Payload[0] << 21) ^ v.Payload[6])
         * 0x9e3779b97f4a7c15ull;
}
static u64 Fingerprint(const std::string& v)
{
    return std::hash<std::string>{}(v);
}

// ============================================================================
// Comparators (the same object is handed to both sorts)

struct LessCmp
{
    template <typename T>
    bool operator()(const T& a, const T& b) const
    {
        return a < b;
    }
};
struct GreaterCmp
{
    template <typename T>
    bool operator()(const T& a, const T& b) const
    {
        return a > b;
    }
};

[[gnu::noinline]] static bool IntLessFn(const int& a, const int& b)
{
    return a < b;
}
using IntFnPtr = bool (*)(const int&, const int&);

template <typename Cmp>
struct CountingCmp
{
    Cmp   Inner;
    u64*  Count;

    template <typename T>
    bool operator()(const T& a, const T& b) const
    {
        ++*Count;
        return Inner(a, b);
    }
};

// ============================================================================
// Input patterns. Each one produces ranks in [0, n); a type case turns a
// rank into an element.

using Ranks = std::vector<u64>;

static std::mt19937_64& Rng()
{
    static std::mt19937_64 rng(0x5eed1234abcdull);
    return rng;
}
static u64 RandomBelow(u64 bound)
{
    return bound ? std::uniform_int_distribution<u64>(0, bound - 1)(Rng())
                 : 0;
}

static Ranks Iota(std::size_t n)
{
    Ranks r(n);
    std::iota(r.begin(), r.end(), u64{0});
    return r;
}

static Ranks GenRandom(std::size_t n)
{
    auto r = Iota(n);
    std::shuffle(r.begin(), r.end(), Rng());
    return r;
}
static Ranks GenRandomDups(std::size_t n)
{
    Ranks r(n);
    for (auto& v : r) v = RandomBelow(n);
    return r;
}
static Ranks GenSorted(std::size_t n) { return Iota(n); }
static Ranks GenReverse(std::size_t n)
{
    auto r = Iota(n);
    std::reverse(r.begin(), r.end());
    return r;
}
static Ranks GenEqual(std::size_t n) { return Ranks(n, n / 2); }
static Ranks GenFewUnique(std::size_t n)
{
    Ranks r(n);
    for (auto& v : r) v = RandomBelow(8) * n / 8;
    return r;
}
static Ranks GenTwoValues(std::size_t n)
{
    Ranks r(n);
    for (auto& v : r) v = RandomBelow(2) * (n / 2);
    return r;
}
static Ranks GenSqrtUnique(std::size_t n)
{
    u64   buckets = std::max<u64>(1, static_cast<u64>(std::sqrt(double(n))));
    Ranks r(n);
    for (auto& v : r) v = RandomBelow(buckets) * n / buckets;
    return r;
}
static Ranks GenSkewed(std::size_t n)
{
    // Most values crowd near zero: rank = n * u^4.
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    Ranks                                  r(n);
    for (auto& v : r)
    {
        double u = unit(Rng());
        v        = std::min<u64>(n ? n - 1 : 0, u64(double(n) * u * u * u * u));
    }
    return r;
}
static void RandomSwaps(Ranks& r, std::size_t swaps)
{
    for (std::size_t i = 0; i < swaps && r.size() > 1; ++i)
        std::swap(r[RandomBelow(r.size())], r[RandomBelow(r.size())]);
}
static Ranks GenNearlySorted(std::size_t n)
{
    auto r = Iota(n);
    RandomSwaps(r, std::max<std::size_t>(1, n / 100));
    return r;
}
static Ranks GenNearlyReverse(std::size_t n)
{
    auto r = GenReverse(n);
    RandomSwaps(r, std::max<std::size_t>(1, n / 100));
    return r;
}
static Ranks GenSortedTail(std::size_t n)
{
    // Sorted, then the last 1% replaced by random values.
    auto        r    = Iota(n);
    std::size_t tail = std::max<std::size_t>(1, n / 100);
    for (std::size_t i = n > tail ? n - tail : 0; i < n; ++i)
        r[i] = RandomBelow(n);
    return r;
}
static Ranks GenPushFront(std::size_t n)
{
    // 1, 2, ..., n-1, 0: one element out of place at the end.
    Ranks r(n);
    for (std::size_t i = 0; i < n; ++i) r[i] = (i + 1) % std::max<u64>(n, 1);
    return r;
}
static Ranks GenPushMiddle(std::size_t n)
{
    // Sorted, with the maximum moved to the middle.
    auto r = Iota(n);
    if (n > 1) std::rotate(r.begin() + n / 2, r.end() - 1, r.end());
    return r;
}
static Ranks GenOrganPipe(std::size_t n)
{
    Ranks r(n);
    for (std::size_t i = 0; i < n; ++i) r[i] = i < n / 2 ? 2 * i : 2 * (n - i) - 1;
    return r;
}
static Ranks GenSawtooth(std::size_t n)
{
    // Ascending runs of length sqrt(n).
    u64   run = std::max<u64>(1, static_cast<u64>(std::sqrt(double(n))));
    Ranks r(n);
    for (std::size_t i = 0; i < n; ++i) r[i] = (i % run) * (n / run) + i / run;
    for (auto& v : r) v = std::min<u64>(v, n - 1);
    return r;
}
static Ranks GenReverseSawtooth(std::size_t n)
{
    auto r = GenSawtooth(n);
    for (auto& v : r) v = n - 1 - v;
    return r;
}
static Ranks GenInterleaved(std::size_t n)
{
    // Even slots ascend, odd slots descend.
    Ranks r(n);
    for (std::size_t i = 0; i < n; ++i) r[i] = i % 2 == 0 ? i : n - i;
    for (auto& v : r) v = std::min<u64>(v, n - 1);
    return r;
}
static Ranks GenRandomRuns(std::size_t n)
{
    // A random permutation cut into sorted blocks of 32.
    auto r = GenRandom(n);
    for (std::size_t i = 0; i < n; i += 32)
        std::sort(r.begin() + i, r.begin() + std::min(n, i + 32));
    return r;
}
static Ranks GenMedianOf3Killer(std::size_t n)
{
    // Musser's sequence against median-of-three (first, middle, last)
    // pivots. Defined for even n; an odd n gets the largest value appended.
    std::size_t k = n / 2;
    Ranks       r(n);
    for (std::size_t i = 1; i <= k; ++i)
    {
        if (i % 2 == 1)
        {
            r[i - 1] = i;
            r[i]     = k + i;
        }
        r[k + i - 1] = 2 * i;
    }
    if (n % 2) r[n - 1] = n;
    for (auto& v : r) v = v ? v - 1 : 0;
    return r;
}

// McIlroy's "A Killer Adversary for Quicksort": the comparator decides the
// values lazily, in the way that hurts the sort under test the most. The
// values it settles on are then a fixed bad input for that sort.
template <typename SortFn>
static Ranks Antiqsort(std::size_t n, SortFn sort)
{
    struct State
    {
        std::vector<u64> Value;
        u64              Gas;
        u64              Solid     = 0;
        std::size_t      Candidate = 0;
    } state{std::vector<u64>(n, n), n};

    auto cmp = [s = &state](std::size_t x, std::size_t y)
    {
        auto& v = s->Value;
        if (v[x] == s->Gas && v[y] == s->Gas)
        {
            if (x == s->Candidate) v[x] = s->Solid++;
            else v[y] = s->Solid++;
        }
        if (v[x] == s->Gas) s->Candidate = x;
        else if (v[y] == s->Gas) s->Candidate = y;
        return v[x] < v[y];
    };

    std::vector<std::size_t> index(n);
    std::iota(index.begin(), index.end(), std::size_t{0});
    sort(index.data(), index.data() + n, cmp);

    for (auto& v : state.Value)
        if (v == state.Gas) v = state.Solid++;
    return state.Value;
}
static Ranks GenAntiqsortPrism(std::size_t n)
{
    return Antiqsort(n, [](auto first, auto last, auto cmp)
                     { Prism::Sort(first, last, cmp); });
}
static Ranks GenAntiqsortStd(std::size_t n)
{
    return Antiqsort(n, [](auto first, auto last, auto cmp)
                     { std::sort(first, last, cmp); });
}

struct Pattern
{
    const char* Name;
    Ranks (*Generate)(std::size_t);
};

static const Pattern s_Patterns[] = {
    {"random", GenRandom},
    {"random_dups", GenRandomDups},
    {"sqrt_unique", GenSqrtUnique},
    {"few_unique", GenFewUnique},
    {"two_values", GenTwoValues},
    {"all_equal", GenEqual},
    {"skewed", GenSkewed},
    {"sorted", GenSorted},
    {"reverse", GenReverse},
    {"nearly_sorted", GenNearlySorted},
    {"nearly_reverse", GenNearlyReverse},
    {"sorted_tail", GenSortedTail},
    {"push_front", GenPushFront},
    {"push_middle", GenPushMiddle},
    {"organ_pipe", GenOrganPipe},
    {"sawtooth", GenSawtooth},
    {"rev_sawtooth", GenReverseSawtooth},
    {"interleaved", GenInterleaved},
    {"random_runs", GenRandomRuns},
    {"med3_killer", GenMedianOf3Killer},
    {"antiqsort_prism", GenAntiqsortPrism},
    {"antiqsort_std", GenAntiqsortStd},
};

// ============================================================================
// Measurement

struct Options
{
    std::vector<std::size_t> Sizes
        = {4, 16, 64, 256, 1000, 10000, 100000, 1000000};
    std::vector<std::string> Patterns;
    std::vector<std::string> Types;
    double                   MinTimeMs = 30.0;
    int                      MinReps   = 5;
    int                      MaxReps   = 101;
    std::string              CsvPath;
    bool                     Color = false;
};

struct Result
{
    std::size_t Size;
    std::string Pattern;
    std::string Type;
    double      PrismNs; // per element, median
    double      StdNs;
    double      PrismCmps; // comparisons per element
    double      StdCmps;
    bool        Ok;
    std::string Error;

    double      Ratio() const { return StdNs > 0 ? PrismNs / StdNs : 1.0; }
};

static double Median(std::vector<double> v)
{
    std::sort(v.begin(), v.end());
    std::size_t m = v.size() / 2;
    return v.size() % 2 ? v[m] : (v[m - 1] + v[m]) / 2;
}

template <typename T, typename Cmp>
struct TypeCase
{
    const char* Name;
    Cmp         Comparator;
    T (*Make)(u64 rank, std::size_t n, std::size_t index);
};

template <typename T, typename Cmp>
static Result RunCase(const Options& opt, const TypeCase<T, Cmp>& tc,
                      const Pattern& pattern, const Ranks& ranks)
{
    const std::size_t n = ranks.size();
    Result            res{n, pattern.Name, tc.Name, 0, 0, 0, 0, true, {}};

    std::vector<T>    input;
    input.reserve(n);
    for (std::size_t i = 0; i < n; ++i) input.push_back(tc.Make(ranks[i], n, i));

    // Small inputs are sorted in batches of copies so a timed region is
    // long enough for the clock; the copies are refilled outside it.
    const std::size_t batch = std::max<std::size_t>(1, (1u << 15) / std::max<std::size_t>(n, 1));
    std::vector<T>    work(n * batch);
    auto              refill = [&]
    {
        for (std::size_t b = 0; b < batch; ++b)
            std::copy(input.begin(), input.end(), work.begin() + b * n);
    };

    auto prismSort = [&](T* first, T* last, auto cmp)
    { Prism::Sort(first, last, cmp); };
    auto stdSort = [&](T* first, T* last, auto cmp)
    { std::sort(first, last, cmp); };

    auto timeOnce = [&](auto sort) -> double
    {
        refill();
        auto start = Clock::now();
        for (std::size_t b = 0; b < batch; ++b)
            sort(work.data() + b * n, work.data() + (b + 1) * n, tc.Comparator);
        auto end = Clock::now();
        return std::chrono::duration<double, std::nano>(end - start).count();
    };

    // Correctness and comparison counts first (also warms the caches).
    auto check = [&](auto sort, double& cmpsPerElem) -> std::vector<T>
    {
        std::vector<T> out = input;
        u64            count = 0;
        sort(out.data(), out.data() + n, CountingCmp<Cmp>{tc.Comparator, &count});
        cmpsPerElem = n ? double(count) / double(n) : 0;
        return out;
    };
    auto prismOut = check(prismSort, res.PrismCmps);
    auto stdOut   = check(stdSort, res.StdCmps);

    u64  inFp = 0, prismFp = 0;
    for (std::size_t i = 0; i < n; ++i)
    {
        inFp += Fingerprint(input[i]);
        prismFp += Fingerprint(prismOut[i]);
    }
    for (std::size_t i = 0; i < n && res.Ok; ++i)
    {
        const auto& a = prismOut[i];
        const auto& b = stdOut[i];
        if (tc.Comparator(a, b) || tc.Comparator(b, a))
        {
            res.Ok    = false;
            res.Error = "differs from std::sort at index " + std::to_string(i);
        }
    }
    if (res.Ok && inFp != prismFp)
    {
        res.Ok    = false;
        res.Error = "elements lost or duplicated";
    }

    // Timed runs, alternating which sort goes first.
    std::vector<double> prismTimes, stdTimes;
    double              total = 0;
    const double        minNs = opt.MinTimeMs * 1e6;
    for (int rep = 0; rep < opt.MaxReps; ++rep)
    {
        double p, s;
        if (rep % 2 == 0)
        {
            p = timeOnce(prismSort);
            s = timeOnce(stdSort);
        }
        else
        {
            s = timeOnce(stdSort);
            p = timeOnce(prismSort);
        }
        prismTimes.push_back(p);
        stdTimes.push_back(s);
        total += std::min(p, s);
        if (rep + 1 >= opt.MinReps && total >= minNs) break;
    }

    const double elems = double(std::max<std::size_t>(n, 1) * batch);
    res.PrismNs        = Median(prismTimes) / elems;
    res.StdNs          = Median(stdTimes) / elems;
    return res;
}

// ============================================================================
// Type cases

static std::uint8_t MakeU8(u64 rank, std::size_t n, std::size_t)
{
    // Scale to the 0-255 range so the pattern keeps its shape.
    return static_cast<std::uint8_t>(n ? rank * 256 / n : 0);
}
static int    MakeI32(u64 rank, std::size_t, std::size_t) { return int(rank); }
static i64    MakeI64(u64 rank, std::size_t, std::size_t)
{
    // Spread over the whole range, negatives included, order preserved.
    return i64(rank * 0x9e3779b9ull) - (i64(1) << 62);
}
static double MakeF64(u64 rank, std::size_t, std::size_t)
{
    return double(rank) * 0.731 - 1e5;
}
static Pair16 MakePair16(u64 rank, std::size_t, std::size_t index)
{
    return {rank, index};
}
static Big64 MakeBig64(u64 rank, std::size_t, std::size_t index)
{
    Big64 v{};
    v.Key = rank;
    for (u64 i = 0; i < 7; ++i) v.Payload[i] = index * 7 + i;
    return v;
}
static std::string MakeStr(u64 rank, std::size_t, std::size_t)
{
    return MakeString(rank);
}

static const TypeCase<std::uint8_t, LessCmp> s_U8{"u8", {}, MakeU8};
static const TypeCase<int, LessCmp>          s_I32{"i32", {}, MakeI32};
static const TypeCase<int, GreaterCmp>       s_I32Desc{"i32_greater", {}, MakeI32};
static const TypeCase<int, IntFnPtr>   s_I32FnPtr{"i32_fnptr", IntLessFn, MakeI32};
static const TypeCase<i64, LessCmp>    s_I64{"i64", {}, MakeI64};
static const TypeCase<double, LessCmp> s_F64{"f64", {}, MakeF64};
static const TypeCase<Pair16, LessCmp> s_Pair16{"pair16", {}, MakePair16};
static const TypeCase<Big64, LessCmp>  s_Big64{"big64", {}, MakeBig64};
static const TypeCase<std::string, LessCmp> s_Str{"string", {}, MakeStr};

static const char* const s_TypeNames[]
    = {"u8",     "i32",   "i32_greater", "i32_fnptr", "i64",
       "f64",    "pair16", "big64",      "string"};

template <typename F>
static void ForEachType(F&& f)
{
    f(s_U8);
    f(s_I32);
    f(s_I32Desc);
    f(s_I32FnPtr);
    f(s_I64);
    f(s_F64);
    f(s_Pair16);
    f(s_Big64);
    f(s_Str);
}

// ============================================================================
// Reporting

static bool Selected(const std::vector<std::string>& filter, std::string_view name)
{
    return filter.empty()
        || std::find(filter.begin(), filter.end(), name) != filter.end();
}

static const char* RatioColor(const Options& opt, double ratio)
{
    if (!opt.Color) return "";
    if (ratio > 1.10) return "\033[31m";
    if (ratio < 0.90) return "\033[32m";
    return "";
}
static const char* Reset(const Options& opt) { return opt.Color ? "\033[0m" : ""; }

static void PrintRow(const Options& opt, const Result& r)
{
    std::printf("%9zu  %-16s %-12s %9.2f %9.2f  %s%6.3f%s  %7.2f %7.2f%s%s\n",
                r.Size, r.Pattern.c_str(), r.Type.c_str(), r.PrismNs, r.StdNs,
                RatioColor(opt, r.Ratio()), r.Ratio(), Reset(opt), r.PrismCmps,
                r.StdCmps, r.Ok ? "" : "  FAIL: ", r.Ok ? "" : r.Error.c_str());
    std::fflush(stdout);
}

template <typename Key>
static void PrintGeomeans(const char* title, const std::vector<Result>& results,
                          Key key, const std::vector<std::string>& order,
                          const Options& opt)
{
    std::printf("\n%s (geometric mean of prism/std):\n", title);
    for (const auto& name : order)
    {
        double sumLog = 0;
        int    count = 0;
        double worst = 0, best = 1e300;
        for (const auto& r : results)
        {
            if (key(r) != name) continue;
            sumLog += std::log(r.Ratio());
            worst = std::max(worst, r.Ratio());
            best  = std::min(best, r.Ratio());
            ++count;
        }
        if (!count) continue;
        double g = std::exp(sumLog / count);
        std::printf("  %-16s %s%6.3f%s   best %6.3f  worst %6.3f  (%d cases)\n",
                    name.c_str(), RatioColor(opt, g), g, Reset(opt), best,
                    worst, count);
    }
}

static std::vector<std::string> SplitList(const char* s)
{
    std::vector<std::string> out;
    std::string              cur;
    for (; *s; ++s)
    {
        if (*s == ',')
        {
            if (!cur.empty()) out.push_back(cur);
            cur.clear();
        }
        else cur += *s;
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

static void Usage(const char* argv0)
{
    std::printf(
        "usage: %s [options]\n"
        "  --quick             sizes up to 10000, shorter timing\n"
        "  --sizes A,B,...     element counts (default 4..1000000)\n"
        "  --patterns A,B,...  input patterns (see --list)\n"
        "  --types A,B,...     element types (see --list)\n"
        "  --min-time-ms X     timed work per case and sort (default 30)\n"
        "  --csv FILE          also write every result as CSV\n"
        "  --list              list patterns and types\n",
        argv0);
}

int main(int argc, char** argv)
{
    Options opt;
    opt.Color = isatty(STDOUT_FILENO);

    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg  = argv[i];
        auto             next = [&]() -> const char*
        {
            if (i + 1 >= argc)
            {
                std::fprintf(stderr, "%s needs a value\n", argv[i]);
                std::exit(2);
            }
            return argv[++i];
        };
        if (arg == "--quick")
        {
            opt.Sizes     = {4, 16, 64, 256, 1000, 10000};
            opt.MinTimeMs = 5;
            opt.MinReps   = 3;
        }
        else if (arg == "--sizes")
        {
            opt.Sizes.clear();
            for (auto& s : SplitList(next()))
                opt.Sizes.push_back(std::strtoull(s.c_str(), nullptr, 10));
        }
        else if (arg == "--patterns") opt.Patterns = SplitList(next());
        else if (arg == "--types") opt.Types = SplitList(next());
        else if (arg == "--min-time-ms") opt.MinTimeMs = std::strtod(next(), nullptr);
        else if (arg == "--csv") opt.CsvPath = next();
        else if (arg == "--list")
        {
            std::printf("patterns:");
            for (const auto& p : s_Patterns) std::printf(" %s", p.Name);
            std::printf("\ntypes:");
            for (const char* t : s_TypeNames) std::printf(" %s", t);
            std::printf("\n");
            return 0;
        }
        else
        {
            Usage(argv[0]);
            return arg == "--help" || arg == "-h" ? 0 : 2;
        }
    }

    for (const auto& name : opt.Patterns)
        if (std::none_of(std::begin(s_Patterns), std::end(s_Patterns),
                         [&](const Pattern& p) { return name == p.Name; }))
        {
            std::fprintf(stderr, "unknown pattern '%s' (see --list)\n", name.c_str());
            return 2;
        }
    for (const auto& name : opt.Types)
        if (std::none_of(std::begin(s_TypeNames), std::end(s_TypeNames),
                         [&](const char* t) { return name == t; }))
        {
            std::fprintf(stderr, "unknown type '%s' (see --list)\n", name.c_str());
            return 2;
        }

#ifndef __OPTIMIZE__
    std::printf("warning: built without optimizations, numbers are meaningless\n");
#endif
    std::printf("ns/elem = median time per element; ratio = prism / std "
                "(< 1: Prism faster); cmp/elem = comparisons per element\n\n");
    std::printf("%9s  %-16s %-12s %9s %9s  %6s  %7s %7s\n", "size", "pattern",
                "type", "prism", "std", "ratio", "cmp(P)", "cmp(S)");

    std::vector<Result> results;
    for (std::size_t n : opt.Sizes)
    {
        std::printf("%s\n", std::string(88, '-').c_str());
        for (const auto& pattern : s_Patterns)
        {
            if (!Selected(opt.Patterns, pattern.Name)) continue;
            Ranks ranks = pattern.Generate(n);
            ForEachType(
                [&](const auto& tc)
                {
                    if (!Selected(opt.Types, tc.Name)) return;
                    results.push_back(RunCase(opt, tc, pattern, ranks));
                    PrintRow(opt, results.back());
                });
        }
    }

    // Summaries.
    std::vector<std::string> patternOrder, typeOrder, sizeOrder;
    for (const auto& p : s_Patterns) patternOrder.push_back(p.Name);
    for (const char* t : s_TypeNames) typeOrder.push_back(t);
    for (std::size_t n : opt.Sizes) sizeOrder.push_back(std::to_string(n));

    PrintGeomeans("By pattern", results, [](const Result& r) { return r.Pattern; },
                  patternOrder, opt);
    PrintGeomeans("By type", results, [](const Result& r) { return r.Type; },
                  typeOrder, opt);
    PrintGeomeans("By size", results,
                  [](const Result& r) { return std::to_string(r.Size); },
                  sizeOrder, opt);
    PrintGeomeans("Overall", results, [](const Result&) { return std::string("all"); },
                  {"all"}, opt);

    int faster = 0, slower = 0, even = 0, failed = 0;
    for (const auto& r : results)
    {
        if (!r.Ok) ++failed;
        if (r.Ratio() < 0.97) ++faster;
        else if (r.Ratio() > 1.03) ++slower;
        else ++even;
    }
    std::printf("\nPrism faster in %d, slower in %d, within 3%% in %d of %zu cases\n",
                faster, slower, even, results.size());

    std::vector<const Result*> sorted;
    for (const auto& r : results) sorted.push_back(&r);
    std::sort(sorted.begin(), sorted.end(),
              [](const Result* a, const Result* b) { return a->Ratio() > b->Ratio(); });
    std::printf("\nWorst 15 for Prism:\n");
    for (std::size_t i = 0; i < std::min<std::size_t>(15, sorted.size()); ++i)
        PrintRow(opt, *sorted[i]);
    std::printf("\nBest 15 for Prism:\n");
    for (std::size_t i = 0; i < std::min<std::size_t>(15, sorted.size()); ++i)
        PrintRow(opt, *sorted[sorted.size() - 1 - i]);

    if (!opt.CsvPath.empty())
    {
        if (FILE* f = std::fopen(opt.CsvPath.c_str(), "w"))
        {
            std::fprintf(f, "size,pattern,type,prism_ns_per_elem,std_ns_per_elem,"
                            "ratio,prism_cmps_per_elem,std_cmps_per_elem,ok\n");
            for (const auto& r : results)
                std::fprintf(f, "%zu,%s,%s,%.4f,%.4f,%.4f,%.3f,%.3f,%d\n", r.Size,
                             r.Pattern.c_str(), r.Type.c_str(), r.PrismNs, r.StdNs,
                             r.Ratio(), r.PrismCmps, r.StdCmps, r.Ok ? 1 : 0);
            std::fclose(f);
            std::printf("\nwrote %s\n", opt.CsvPath.c_str());
        }
        else std::fprintf(stderr, "cannot write %s\n", opt.CsvPath.c_str());
    }

    if (failed)
    {
        std::printf("\n%d case(s) FAILED verification\n", failed);
        return 1;
    }
    return 0;
}
