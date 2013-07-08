/* kmalloc.h - Kmalloc/kfree for Mink kernel.
 *
 * Portions copyright (c)2012 James Molloy.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
#ifndef __MINK_KMALLOC_H
#define __MINK_KMALLOC_H

void *kmalloc(unsigned sz);
void kfree(void *p);

#endif
