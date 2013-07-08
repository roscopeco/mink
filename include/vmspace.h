/* vmspace.h - Virtual memory address space manager for Mink.
 *
 * Portions copyright (c)2012 James Molloy.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
#ifndef __MINK_VMSPACE_H
#define __MINK_VMSPACE_H

#include <stdint.h>
#include "adt/buddy.h"

typedef struct vmspace {
  uintptr_t start;
  uintptr_t size;
  buddy_t allocator;
  spinlock_t lock;
} vmspace_t;

int vmspace_init(vmspace_t *vms, uintptr_t addr, uintptr_t sz);
uintptr_t vmspace_alloc(vmspace_t *vms, unsigned sz, int alloc_phys);
void vmspace_free(vmspace_t *vms, unsigned sz, uintptr_t addr, int free_phys);

extern vmspace_t kernel_vmspace;

#endif
