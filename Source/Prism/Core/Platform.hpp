/*
 * Created by v1tr10l7 on 01.03.2025.
 * Copyright (c) 2024-2025, Szymon Zemke <v1tr10l7@proton.me>
 *
 * SPDX-License-Identifier: GPL-3
 */
#pragma once

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
