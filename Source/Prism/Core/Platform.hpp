/*
 * Created by v1tr10l7 on 01.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

/* =========================
 * Operating System
 * ========================= */
#if defined(_WIN32) || defined(_WIN64)
    #define PM_PLATFORM_WINDOWS 1
#elif defined(__ANDROID__)
    #define PM_PLATFORM_ANDROID 1
#elif defined(__linux__)
    #define PM_PLATFORM_LINUX 1
#elif defined(__APPLE__) || defined(__MACH__)
    #define PM_PLATFORM_APPLE 1
#elif defined(__unix__)
    #define PM_PLATFORM_UNIX 1
#else
    #define PM_PLATFORM_UNKNOWN 1
#endif

#if defined(PM_PLATFORM_APPLE)
    #include <TargetConditionals.h>

    #if TARGET_OS_MAC
        #define PM_PLATFORM_MACOS 1
    #elif TARGET_OS_IPHONE
        #define PM_PLATFORM_IOS 1
    #elif TARGET_IPHONE_SIMULATOR
        #define PM_PLATFORM_IOS_SIMULATOR 1
    #elif TARGET_OS_MACCATALYST
        #define PM_PLATFORM_MACCATALYST 1
    #else
        #define PM_PLATFORM_APPLE_UNKNOWN 1
    #endif
#endif

#ifdef __i386__
    #define PRISM_TARGET_IA32
    #define PRISM_TARGET_STRING "ia32"
#elifdef __x86_64__
    #define PRISM_TARGET_X86_64
    #define PRISM_TARGET_STRING "x86_64"

    #ifdef __SSE2__
        #define PRISM_SIMD_SSE2_PRESENT 1
    #else
        #define PRISM_SIMD_SSE2_PRESENT 0
    #endif
#elifdef __aarch64__
    #define PRISM_TARGET_AARCH64
    #define PRISM_TARGET_STRING "aarch64"

    #if defined(__ARM_NEON) || defined(__ARM_NEON__)
        #define PRISM_SIMD_NEON_PRESENT 1
    #else
        #define PRISM_SIMD_NEON_PRESENT 0
    #endif
#elif defined(__riscv) && __riscv_xlen == 64
    #define PRISM_TARGET_RISCV64
    #define PRISM_TARGET_STRING "riscv64"
#elifdef __loongarch64
    #define PRISM_TARGET_LOONGARCH64
    #define PRISM_TARGET_STRING "loongarch64"
#endif

/* =========================
 * Endianness
 * ========================= */

#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__)                \
    && defined(__ORDER_BIG_ENDIAN__)
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define PM_ENDIAN_LITTLE 1
    #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
        #define PM_ENDIAN_BIG 1
    #endif
#elif defined(_WIN32)
    #define PM_ENDIAN_LITTLE 1
#else
    #define PM_ENDIAN_UNKNOWN 1
#endif
/* =========================
 * ABI
 * ========================= */
#if defined(PM_PLATFORM_WINDOWS)
    #define PM_ABI_MS 1
#elif defined(__ELF__)
    #define PM_ABI_SYSV 1
#else
    #define PM_ABI_UNKNOWN 1
#endif
/* =========================
 * Build Context
 * ========================= */
#if defined(PRISM_BUILD_CRYPTIX_KERNEL)
    #define PM_CONTEXT_KERNEL 1
#else
    #define PM_CONTEXT_USERSPACE 1
#endif

/* =========================
 * Symbol visibility
 * ========================= */
#if defined(PM_PLATFORM_WINDOWS)
/* Windows (MSVC, Clang-CL, MinGW) */
    #if defined(PM_COMPILER_MSVC)
        #define PM_EXPORT __declspec(dllexport)
        #define PM_IMPORT __declspec(dllimport)
    #else
        /* GCC / Clang on Windows */
        #define PM_EXPORT __attribute__((dllexport))
        #define PM_IMPORT __attribute__((dllimport))
    #endif
#else
/* ELF / Mach-O (Linux, BSD, macOS, etc.) */
    #if defined(PM_COMPILER_GCC) || defined(PM_COMPILER_CLANG)
        #define PM_EXPORT __attribute__((visibility("default")))
        #define PM_IMPORT
    #else
        /* Unknown compiler */
        #define PM_EXPORT
        #define PM_IMPORT
    #endif
#endif
