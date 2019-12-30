/*
 * x86_64/mmap.h - x86_64-specific mmap.h for Mink.
 *
 * Copyright (c)2018 Ross Bamford.
 *
 * See LICENSE for details.
 */
#ifndef __MINK_X86_64_MMAP_H
#define __MINK_X86_64_MMAP_H

#define MMAP_KERNEL_START 0xFFFFFFFF80000000

#define MMAP_KERNEL_VMSPACE_START \
                          0xFFFFFFFF90000000
#define MMAP_KERNEL_VMSPACE_END \
                          0xFFFFFFFFFE800000

#define MMAP_PMM_BITMAP   0xFFFFFFFFFE800000
#define MMAP_PMM_BITMAP_END 0xFFFFFFFFFF800000

#define MMAP_KERNEL_END   0xFFFFFFFFFF800000

#endif
