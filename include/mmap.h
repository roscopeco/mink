/*
 * mmap.h - Memory map definition for Mink.
 *
 * Copyright (c)2013-2019 Ross Bamford.
 *
 * See LICENSE for details.
 *
 * Architecture-specific implementation headers in $(ARCH)/mmap.h must define
 * at least one macro:
 *
 * #define MMAP_KERNEL_START <start address of kernel virtual memory>
 *
 * They will of course define any other macros needed by the architecture-
 * specific code.
 */
#ifndef MMAP_H
#define MMAP_H

#if defined(x86_64)
#include "x86_64/mmap.h"
#elif defined(x86)
#include "x86/mmap.h"
#endif

#define IS_KERNEL_ADDR(x) ((void*)(x) >= (void*)MMAP_KERNEL_START)

#endif
