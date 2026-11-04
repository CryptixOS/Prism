/*
 * Created by v1tr10l7 on 13.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <cassert>

#if PRISM_TEST_MODULES
// Headers (std, fmt, macros) must come before any import.
#include <Prism/Macros.hpp>
import Prism;
#else
#include <Prism/Utility/Path.hpp>
#endif

using namespace Prism;
using namespace Prism::PathLiterals;

void Path_TestConstructorsAndAssignments()
{
    // 1. Default Construction
    Path p1;
    assert(p1.Empty());
    assert(p1.Size() == 0);

    // 2. C-String buffer forwarding constructor
    Path p2("/usr/local/bin");
    assert(p2.Size() == 14);
    assert(!p2.Empty());

    // 3. StringView forwarding
    StringView sv("/var/log");
    Path p3(sv);
    assert(p3.Size() == 8);

    // 4. Copy/Move verification
    Path original("/etc/fstab");
    Path copy(original);
    assert(copy == original);

    Path moved(Move(original));
    assert(moved == "/etc/fstab");

    // 5. Native assignments
    Path assigned;
    assigned = "/opt";
    assert(assigned == "/opt");
}

void Path_TestComponentIteratorsForward()
{
    // Absolute nested path layout
    Path p("/usr/local/bin");
    auto it = p.begin();
    
    // 1. Root component evaluation
    assert(it != p.end());
    assert(*it == "/"_pv);
    
    // 2. Nested directory component indexing
    ++it;
    assert(it != p.end());
    assert(*it == "usr"_pv);
    
    it++;
    assert(it != p.end());
    assert(*it == "local"_pv);
    
    // 3. Trailing file component target
    ++it;
    assert(it != p.end());
    assert(*it == "bin"_pv);
    
    ++it;
    assert(it == p.end());
}

void Path_TestComponentIteratorsBidirectional()
{
    Path p("/usr/share/doc");
    auto it = p.end();

    // 1. Step backward from end placeholder
    --it;
    assert(*it == "doc"_pv);

    it--;
    assert(*it == "share"_pv);

    --it;
    assert(*it == "usr"_pv);

    // 2. Identify the absolute separator tracking limit
    --it;
    assert(*it == "/"_pv);
    assert(it == p.begin());
}

void Path_TestIteratorEdgeCases()
{
    // 1. Completely empty or naked relative strings
    Path emptyPath("");
    assert(emptyPath.begin() == emptyPath.end());

    Path relativePath("relative");
    auto relIt = relativePath.begin();
    assert(*relIt == "relative"_pv);
    ++relIt;
    assert(relIt == relativePath.end());

    // 2. Redundant, sequential token parsing blocks ("///")
    Path multiSlash("///usr///bin///");
    auto it = multiSlash.begin();
    
    assert(*it == "/"_pv);
    ++it;
    assert(*it == "usr"_pv);
    ++it;
    assert(*it == "bin"_pv);
    ++it;
    assert(it == multiSlash.end());

    // 3. Direct execution check via structural reverse iterators
    // Path revPath("/opt/git");
    // auto rit = revPath.crbegin();
    // assert(*rit == "git"_pv);
    // ++rit;
    // assert(*rit == "opt"_pv);
    // ++rit;
    // assert(*rit == "/"_pv);
    // ++rit;
    // assert(rit == revPath.crend());
}

void Path_TestElementAccessAndCapacity()
{
    Path p("/sys/kernel");

    assert(p[0] == '/');
    assert(p.At(5) == 'k');
    assert(p.Front() == '/');
    assert(p.Back() == 'l');
    
    assert(p.Size() == 11);
    assert(!p.Empty());
}

void Path_TestConcatenationAndAppends()
{
    // 1. Path serialization via string appends (operator+=)
    Path p1("/usr");
    p1 += "bin"; 
    assert(p1 == "/usrbin"); // Continuous raw expansion without structural adjustments
    
    p1 += '/';
    assert(p1 == "/usrbin/");

    // 2. Native operational join rules (operator/=)
    Path p2("/usr");
    p2 /= "bin";
    assert(p2 == "/usr/bin"); // Implicit separator safety insert

    Path p3("/var/");
    p3 /= "log";
    assert(p3 == "/var/log"); // Avoids creating duplicate directory markings ("//")

    // 3. Binary generation expressions
    Path base("/home/user");
    Path sub("downloads");
    Path combined = base / sub;
    assert(combined == "/home/user/downloads");
    assert(base == "/home/user"); // Mutation-free verification
}

void Path_TestComparisonsAndQueries()
{
    Path lhs("/etc/nginx");
    Path rhs("/etc/nginx");
    Path differential("/etc/systemd");

    // Equality matches
    assert(lhs == rhs);
    assert(lhs != differential);
    assert(lhs == "/etc/nginx");

    // Three-way ordering checks
    assert((lhs <=> differential) < 0);
    assert((differential <=> lhs) > 0);
    
    // Absolute contextual properties
    assert(lhs.Absolute());
    Path relative("var/log");
    assert(!relative.Absolute());
}

void Path_TestStringOperations()
{
    Path p("/var/www/index.html");

    // Substring parsing forward logic
    StringView sub = p.Substr(5, 3);
    assert(sub == "www");

    // Affix scanning rules
    assert(p.StartsWith("/var"));
    assert(p.EndsWith(".html"));
    assert(p.Contains("www"));
}

#if 1
void Path_TestHashSpecialization()
{
#if PRISM_DISABLE_STDHASH == 0
    Hash<Path> hasher;
    Path p1("/usr/local"_p);
    Path p2("/usr/local"_p);
    Path p3("/usr/bin"_p);

    assert(hasher(p1) == hasher(p2));
    assert(hasher(p1) != hasher(p3));
    assert(IsFastHash<Hash<Path>>::Value == false);
#endif
}
#endif

int main()
{
    Path_TestConstructorsAndAssignments();
    Path_TestComponentIteratorsForward();
    Path_TestComponentIteratorsBidirectional();
    Path_TestIteratorEdgeCases();
    Path_TestElementAccessAndCapacity();
    Path_TestConcatenationAndAppends();
    Path_TestComparisonsAndQueries();
    Path_TestStringOperations();
    Path_TestHashSpecialization();

    return 0;
}
