/*
 * Created by v1tr10l7 on 14.11.2026.
 * Copyright (c) 2024-2026, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/**
 * The platform hook contract: everything a freestanding consumer (the
 * CryptixOS kernel, PRISM_TARGET=cryptix) has to define for libprism.a to
 * link. CI checks it: Meta/ci/check-hooks.sh compares the undefined symbols
 * of a freestanding libprism.a with Meta/ci/platform-hooks-<target>.txt.
 *
 * Hosted builds (app, cryptix-app) get all of these from libc, the C++
 * runtime and Source/Prism/Debug/Assertions.cpp / Log.cpp.
 *
 * ## C runtime
 *
 *   memcpy, memset, strlen           the compiler emits calls to them
 *   __stack_chk_fail                 with -fstack-protector*, which many
 *                                    toolchains (and PRISM_HARDENED) enable
 *
 * ## C++ runtime
 *
 *   operator new(usize), operator new[](usize)
 *   operator delete(void*, usize), operator delete[](void*)
 *                                    allocation; Prism never calls malloc.
 *                                    Placement new comes from
 *                                    Core/LanguageSupport.hpp unless
 *                                    PRISM_NO_PLACEMENT_NEW=1.
 *   std::terminate()                 only if the consumer's code needs it
 *
 * ## Prism hooks
 *
 * They take a Prism::StringView (module Prism.String.StringView), so they are
 * declared in modules, not here:
 *
 *   isize Logger::Print(Prism::StringView text)
 *       Global namespace (Prism.Debug.Log). Writes log output, returns the
 *       number of bytes written or a negative value.
 *       carbonc logs with write(1, data, size) instead.
 *
 *   [[noreturn]] void panic(Prism::StringView message)
 *   void earlyPanic(Prism::StringView message)
 *       Global namespace (Prism.Debug.Assertions). Called by PrismPanic,
 *       failed PrismAssert*, and PrismCheck in hardened builds. earlyPanic is
 *       for before the logger works; it may return only if it can't stop.
 *       Hosted builds use Prism::panic / Prism::earlyPanic from
 *       Assertions.cpp, which log and trap.
 *
 *   __assert_fail(const char* expr, const char* file, unsigned line,
 *                 const char* function)
 *       Only without <assert.h>: Types.hpp's assert() calls it. Assertions.cpp
 *       defines it on top of PrismAssert.
 *
 * Time, threads and files are not needed by the core library.
 */

#include <Prism/Core/Compiler.hpp>

extern "C"
{
    void*        memcpy(void* destination, const void* source,
                        __SIZE_TYPE__ count);
    void*        memset(void* destination, int value, __SIZE_TYPE__ count);
    __SIZE_TYPE__ strlen(const char* string);
    PM_NORETURN void __stack_chk_fail();
}
