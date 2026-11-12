/*
 * Created by v1tr10l7 on 13.06.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include <Prism/Test/Test.hpp>
import Prism.Test.Test;
import Prism.Utility.Path;

using namespace Prism;
using namespace Prism::PathLiterals;

TEST_CASE(Path_TestConstructorsAndAssignments){
    // 1. Default Construction
    Path p1;
    EXPECT(p1.Empty());
    EXPECT_EQ(p1.Size(), 0);

    // 2. C-String buffer forwarding constructor
    Path p2("/usr/local/bin");
    EXPECT_EQ(p2.Size(), 14);
    EXPECT(!p2.Empty());

    // 3. StringView forwarding
    StringView sv("/var/log");
    Path p3(sv);
    EXPECT_EQ(p3.Size(), 8);

    // 4. Copy/Move verification
    Path original("/etc/fstab");
    Path copy(original);
    EXPECT_EQ(copy, original);

    Path moved(Move(original));
    EXPECT_EQ(moved, ("/etc/fstab"));

    // 5. Native assignments
    Path assigned;
    assigned = "/opt";
    EXPECT_EQ(assigned, ("/opt"));
}

TEST_CASE(Path_TestComponentIteratorsForward){
    // Absolute nested path layout
    Path p("/usr/local/bin");
    auto it = p.begin();
    
    // 1. Root component evaluation
    EXPECT_NE(it, p.end());
    EXPECT_EQ((*it), ("/"_pv));
    
    // 2. Nested directory component indexing
    ++it;
    EXPECT_NE(it, p.end());
    EXPECT_EQ((*it), "usr"_pv);
    
    it++;
    EXPECT_NE(it, p.end());
    EXPECT_EQ((*it), "local"_pv);
    
    // 3. Trailing file component target
    ++it;
    EXPECT_NE(it, p.end());
    EXPECT_EQ((*it), "bin"_pv);
    
    ++it;
    EXPECT_EQ(it, p.end());
}

TEST_CASE(Path_TestComponentIteratorsBidirectional){
    Path p("/usr/share/doc");
    auto it = p.end();

    // 1. Step backward from end placeholder
    --it;
    EXPECT_EQ((*it), "doc"_pv);

    it--;
    EXPECT_EQ((*it), "share"_pv);

    --it;
    EXPECT_EQ((*it), "usr"_pv);

    // 2. Identify the absolute separator tracking limit
    --it;
    EXPECT_EQ((*it), ("/"_pv));
    EXPECT_EQ(it, p.begin());
}

TEST_CASE(Path_TestIteratorEdgeCases){
    // 1. Completely empty or naked relative strings
    Path emptyPath("");
    EXPECT_EQ(emptyPath.begin(), emptyPath.end());

    Path relativePath("relative");
    auto relIt = relativePath.begin();
    EXPECT_EQ((*relIt), "relative"_pv);
    ++relIt;
    EXPECT_EQ(relIt, relativePath.end());

    // 2. Redundant, sequential token parsing blocks ("///")
    Path multiSlash("///usr///bin///");
    auto it = multiSlash.begin();
    
    EXPECT_EQ((*it), ("/"_pv));
    ++it;
    EXPECT_EQ((*it), "usr"_pv);
    ++it;
    EXPECT_EQ((*it), "bin"_pv);
    ++it;
    EXPECT_EQ(it, multiSlash.end());

    // 3. Direct execution check via structural reverse iterators
    // Path revPath("/opt/git");
    // auto rit = revPath.crbegin();
    // EXPECT_EQ((*rit), "git"_pv);
    // ++rit;
    // EXPECT_EQ((*rit), "opt"_pv);
    // ++rit;
    // EXPECT_EQ((*rit), ("/"_pv));
    // ++rit;
    // EXPECT_EQ(rit, revPath.crend());
}

TEST_CASE(Path_TestElementAccessAndCapacity){
    Path p("/sys/kernel");

    EXPECT_EQ(p[0], ('/'));
    EXPECT_EQ(p.At(5), 'k');
    EXPECT_EQ(p.Front(), ('/'));
    EXPECT_EQ(p.Back(), 'l');
    
    EXPECT_EQ(p.Size(), 11);
    EXPECT(!p.Empty());
}

TEST_CASE(Path_TestConcatenationAndAppends){
    // 1. Path serialization via string appends (operator+=)
    Path p1("/usr");
    p1 += "bin"; 
    EXPECT_EQ(p1, ("/usrbin")); // Continuous raw expansion without structural adjustments
    
    p1 += '/';
    EXPECT_EQ(p1, ("/usrbin/"));

    // 2. Native operational join rules (operator/=)
    Path p2("/usr");
    p2 /= "bin";
    EXPECT_EQ(p2, ("/usr/bin")); // Implicit separator safety insert

    Path p3("/var/");
    p3 /= "log";
    EXPECT_EQ(p3, ("/var/log")); // Avoids creating duplicate directory markings ("//")

    // 3. Binary generation expressions
    Path base("/home/user");
    Path sub("downloads");
    Path combined = base / sub;
    EXPECT_EQ(combined, ("/home/user/downloads"));
    EXPECT_EQ(base, ("/home/user")); // Mutation-free verification
}

TEST_CASE(Path_TestComparisonsAndQueries){
    Path lhs("/etc/nginx");
    Path rhs("/etc/nginx");
    Path differential("/etc/systemd");

    // Equality matches
    EXPECT_EQ(lhs, rhs);
    EXPECT_NE(lhs, differential);
    EXPECT_EQ(lhs, ("/etc/nginx"));

    // Three-way ordering checks
    EXPECT((lhs <=> differential) < 0);
    EXPECT((differential <=> lhs) > 0);
    
    // Absolute contextual properties
    EXPECT(lhs.Absolute());
    Path relative("var/log");
    EXPECT(!relative.Absolute());
}

TEST_CASE(Path_TestStringOperations){
    Path p("/var/www/index.html");

    // Substring parsing forward logic
    StringView sub = p.Substr(5, 3);
    EXPECT_EQ(sub, "www");

    // Affix scanning rules
    EXPECT(p.StartsWith("/var"));
    EXPECT(p.EndsWith(".html"));
    EXPECT(p.Contains("www"));
}

#if 1
TEST_CASE(Path_TestHashSpecialization){
#if PRISM_DISABLE_STDHASH == 0
    Hash<Path> hasher;
    Path p1("/usr/local"_p);
    Path p2("/usr/local"_p);
    Path p3("/usr/bin"_p);

    EXPECT_EQ(hasher(p1), hasher(p2));
    EXPECT_NE(hasher(p1), hasher(p3));
    EXPECT(IsFastHash<Hash<Path>>::Value == false);
#endif
}
#endif


