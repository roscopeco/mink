/*
 * x86/mmap.h - x86-specific mmap.h for Mink.
 *
 * Copyright (c)2013-2016 Ross Bamford.
 *
 * See LICENSE for details.
 */
#ifndef __MINK_X86_MMAP_H
#define __MINK_X86_MMAP_H

#define MMAP_KERNEL_START 0xC0000000

#define MMAP_COW_REFCNTS  0xCC000000 /* At least 64MB of address space for 36-bit
                                        physical addresses */
#define MMAP_KERNEL_VMSPACE_START \
                          0xD0000000
#define MMAP_KERNEL_VMSPACE_END \
                          0xFE800000

#define MMAP_PMM_BITMAP   0xFE800000
#define MMAP_PMM_BITMAP_END 0xFF800000

#define MMAP_KERNEL_END   0xFF800000

#endif
