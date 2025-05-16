/*
 * Created by v1tr10l7 on 09.05.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */

#include <Prism/Memory/Ref.hpp>
#include <cassert>

using namespace Prism;

struct Dummy : public Prism::RefCounted
{
    int value = 42;
};

void TestRefBasic()
{
    Prism::Ref<Dummy> ref1(new Dummy);
    assert(ref1->value == 42);
    assert(ref1->RefCount() == 1);

    Prism::Ref<Dummy> ref2 = ref1;
    assert(ref1->RefCount() == 2);
    assert(ref2->RefCount() == 2);

    Prism::Ref<Dummy> ref3;
    ref3 = ref1;
    assert(ref3->RefCount() == 3);

    ref2 = nullptr;
    assert(ref1->RefCount() == 2);

    ref3.Reset();
    assert(ref1->RefCount() == 1);
}

void TestRefMove()
{
    Prism::Ref<Dummy> ref1(new Dummy);
    assert(ref1->RefCount() == 1);

    Prism::Ref<Dummy> ref2 = std::move(ref1);
    assert(ref1.Raw() == nullptr);
    assert(ref2->value == 42);
    assert(ref2->RefCount() == 1);
}

void TestRefDeleteOnZero()
{
    bool deleted = false;
    struct Track : Prism::RefCounted
    {
        bool* del;
        Track(bool* d)
            : del(d)
        {
        }
        ~Track() { *del = true; }
    };

    {
        Prism::Ref<Track> ref(new Track(&deleted));
        assert(!deleted);
    }
    assert(deleted); // Ensure destructor was called
}

void TestRefCreateHelper()
{
    auto ref = Prism::CreateRef<Dummy>();
    assert(ref->value == 42);
    assert(ref.RefCount() == 1);
}

int main()
{
    TestRefBasic();
    TestRefMove();
    TestRefDeleteOnZero();
    TestRefCreateHelper();
}
