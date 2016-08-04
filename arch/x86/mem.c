/* mem.c - Memory management feature for Mink kernel.
 *
 * Portions copyright (c)2012 James Molloy.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
#include "hal.h"
#include "sys.h"
#include "utils.h"
#include "x86/multiboot.h"

extern multiboot_info_t mboot;

static void remove_range(range_t *r, uint64_t start, uint64_t extent) {
  /* FIXME: Assumes that a range exists that actually starts at 'start' and
     has extent greater than or equal to 'extent'. */
  if (r->start == start) {
    r->start += extent;
    r->extent -= extent;
  }
}

static int init_memory() {
  if ((mboot.flags & MULTIBOOT_INFO_MEMORY) == 0) {
    panic("Bootloader did not provide memory map info!");
  }
    
  range_t ranges[32];//, ranges_cpy[32];

  uint32_t i = mboot.mmap_addr;
  unsigned n = 0;
  uint64_t extent = 0;
  uint64_t total_len = 0;

  while (i < mboot.mmap_addr+mboot.mmap_length) {
    if (n >= 128) break;

    multiboot_memory_map_t *entry = (multiboot_memory_map_t*)i;

    if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
      ranges[n].start = entry->addr;
      ranges[n++].extent = entry->len;

      if (entry->addr + entry->len > extent)
        extent = entry->addr + entry->len;

      total_len += entry->len;
    }

#if defined(KDEBUG_ENABLED) && defined(KDEBUG_PMM)
    printk("e: sz 0x%08x addr 0x%08x len 0x%08x ty 0x%08x\n", entry->size, (uint32_t)entry->addr, (uint32_t)entry->len, entry->type);
#endif

    i += entry->size + 4;
  }

#ifdef KDEBUG_ENABLED
  printk("Total memory available is %d MiB", total_len / 1024 / 1024);
#endif

  extern int __start, __end;
  uintptr_t end = (((uintptr_t)&__end) & ~get_page_mask()) + get_page_size();

  for (i = 0; i < n; ++i) {
    remove_range(&ranges[i], (uintptr_t)&__start, end);
  }

#if defined(KDEBUG_ENABLED) && defined(KDEBUG_PMM)
  for (i = 0; i < n; ++i) {
    printk("r: 0x%08xx ext 0x%08x\n", (uint32_t)ranges[i].start, (uint32_t)ranges[i].extent);
  }
#endif
  /* Copy the ranges to a backup, as init_physical_memory mutates them and 
     init_cow_refcnts needs to run after init_physical_memory */
  //for (i = 0; i < n; ++i)
  //  ranges_cpy[i] = ranges[i];

  init_physical_memory_early(ranges, n, extent);
  init_virtual_memory(ranges, n);
  init_physical_memory();
  //init_cow_refcnts(ranges_cpy, n);

  return 1;
}

static feature_t x MINK_FEATURE = {
  .name = "x86/mem",
  .required = NULL,
  .load_after = NULL,
  .init = &init_memory,
};
