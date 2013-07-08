/* slab.h - Slab allocator for Mink.
 *
 * Portions copyright (c)2012 James Molloy.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
#ifndef __MINK_SLAB_H
#define __MINK_SLAB_H

#include "vmspace.h"

#define SLAB_SIZE 0x2000

typedef struct slab_cache {
  unsigned size;
  void *init;
  struct slab_footer *first;
  void *empty;
  vmspace_t *vms;

  spinlock_t lock;
} slab_cache_t;

int slab_cache_create(slab_cache_t *c, vmspace_t *vms, unsigned size, void *init);
int slab_cache_destroy(slab_cache_t *c);
void *slab_cache_alloc(slab_cache_t *c);
void slab_cache_free(slab_cache_t *c, void *obj);

#endif
