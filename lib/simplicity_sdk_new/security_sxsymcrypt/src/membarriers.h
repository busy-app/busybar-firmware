/*
 * Copyright (c) 2018-2019 Beerten Engineering scs
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __GNUC__
#define __asm__ asm
#endif


/** Compiler memory barrier.
 *
 * The compiler shall not reorder memory access memory across a call to cmb().
 */
static inline void cmb(void)
{
#ifdef __aarch64__
    __asm__ volatile ("dsb sy;":::"memory");
#else
    __asm__ volatile ("":::"memory");
#endif
}

/** CPU write memory barrier.
 *
 * All write instructions before this call must have been completed by the
 * CPU before the end of this function. No write instruction after this
 * function may be executed by the CPU before this call.
 * This also implies that the compiler may not reorder memory acceses
 * as with cmb().
 */
static inline void wmb(void)
{
#ifdef __aarch64__
    __asm__ volatile ("dsb sy;":::"memory");
#elif defined(__amd64__)
    __asm__ volatile ("sfence;":::"memory");
#else
    __asm__ volatile ("":::"memory");
#endif
}

/** CPU read memory barrier.
 *
 * All read instructions before this call must have been completed by the
 * CPU before the end of this function. No read instruction after this
 * function may be executed by the CPU before this call.
 * This also implies that the compiler may not reorder memory acceses
 * as with cmb().
 */
static inline void rmb(void)
{
#ifdef __aarch64__
    __asm__ volatile ("dsb sy;":::"memory");
#elif defined(__amd64__)
    __asm__ volatile ("lfence;":::"memory");
#else
    __asm__ volatile ("":::"memory");
#endif
}

