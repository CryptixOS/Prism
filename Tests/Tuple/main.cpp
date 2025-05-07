/*
 * Created by v1tr10l7 on 26.04.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#include <Prism/Containers/Tuple.hpp>
#include <Prism/String/String.hpp>
#include <iostream>

using namespace Prism;
using namespace Prism::Literals;

int main()
{
    Tuple tuple{10, 2.50, "Hello"_s};

    std::cout << "Tuple(0) => " << tuple.Get<0>() << "\n";
    std::cout << "Tuple(1) => " << tuple.Get<1>() << "\n";
    std::cout << "Tuple(2) => " << tuple.Get<2>().Raw() << "\n";
}
