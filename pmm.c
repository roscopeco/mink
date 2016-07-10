/* pmm.c - Physical Memory Manager for Mink kernel.
 *
 * Portions copyright (c)2012 James Molloy.
 *
 * The memory management (both pmm and vmm) are exposed to the kernel
 * via the x86/mem feature (defined in x86/mem.c).
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
#include "hal.h"
#include "assert.h"
#include "sys.h"
#include "utils.h"
#include "mmap.h"
#include "adt/buddy.h"

#if defined(KDEBUG_ENABLED) && defined(KDEBUG_PMM)
#define dbg(args...) printk("pmm: " args)
#else
#define dbg(args...)
#endif

#define MIN(x, y) ( (x < y) ? x : y )
#define MAX(x, y) ( (x > y) ? x : y )

#define PMM_INIT_START 0
#define PMM_INIT_EARLY 1
#define PMM_INIT_FULL  2

unsigned pmm_init_stage = PMM_INIT_START;

range_t  early_ranges[64];
unsigned early_nranges;
uint64_t early_max_extent;

static spinlock_t lock = SPINLOCK_RELEASED;
static buddy_t allocators[3];

static range_t split_range(range_t *r, uint64_t loc) {
  range_t ret;

  if (r->start >= loc) {
    ret.extent = 0;
  } else if (r->start + r->extent <= loc) {
    ret = *r;
    r->extent = 0;
  } else {
    ret.start = r->start;
    ret.extent = loc - r->start;

    r->start = loc;
    r->extent = r->extent - ret.extent;
  }

  return ret;
}

uint64_t alloc_page(int req) {
  return alloc_pages(req, 1);
}

uint64_t alloc_pages(int req, size_t num) {
  dbg("alloc_pages: get lock\n");
  spinlock_acquire(&lock);
  dbg("alloc_pages: got lock\n");
  uint64_t val = buddy_alloc(&allocators[req], num * get_page_size());

  if (val == ~0ULL && req == PAGE_REQ_NONE)
    val = buddy_alloc(&allocators[PAGE_REQ_UNDER4GB], num * get_page_size());

  spinlock_release(&lock);
  return val;
}

int free_page(uint64_t page) {
  return free_pages(page, 1);
}

int free_pages(uint64_t pages, size_t num) {
  spinlock_acquire(&lock);

  int req = PAGE_REQ_NONE;
  if (pages < 0x100000)
    req = PAGE_REQ_UNDER1MB;
  else if (pages < 0x100000000ULL)
    req = PAGE_REQ_UNDER4GB;
  
  buddy_free(&allocators[req], pages, num * get_page_size());

  spinlock_release(&lock);
  return 0;
}

uint64_t early_alloc_page() {
  assert(pmm_init_stage == PMM_INIT_EARLY);
  for (unsigned i = 0; i < early_nranges; ++i) {
    if (early_ranges[i].extent <= 0x1000 || early_ranges[i].start >= 0x100000000ULL)
      /* Discard any pages over 4GB physical or less than 4K in size. */
      continue;
    /* Ignore any range under 1MB */
    if (early_ranges[i].start < 0x100000)
      continue;
    
    uint32_t ret = (uint32_t)early_ranges[i].start;
    early_ranges[i].start += 0x1000;
    early_ranges[i].extent -= 0x1000;

    dbg("early_alloc_page() -> %x\n", ret);
    return ret;
  }
  panic("early_alloc_page couldn't find any pages to use!");
  return 0; // dead code, but squashes a warning...
}

int init_physical_memory_early(range_t *ranges, unsigned nranges,
                               uint64_t max_extent) {
  assert(pmm_init_stage == PMM_INIT_START &&
         "init_physical_memory_early() called twice!");
  assert(nranges < 64 && "Too many ranges!");
  memcpy(early_ranges, ranges, nranges * sizeof(range_t));
  early_nranges = nranges;
  early_max_extent = max_extent;

  pmm_init_stage = PMM_INIT_EARLY;
  return 0;
}

int init_physical_memory() {
  if (pmm_init_stage != PMM_INIT_EARLY) {
    panic("init_physical_memory_early not called!");
  }
  
  assert(pmm_init_stage == PMM_INIT_EARLY &&
         "init_physical_memory_early must be called first!");

  range_t rs[3];
  rs[PAGE_REQ_UNDER1MB].start = 0x0;
  rs[PAGE_REQ_UNDER1MB].extent = MAX(MIN(early_max_extent, 0x100000), 0);

  rs[PAGE_REQ_UNDER4GB].start = 0x100000;
  rs[PAGE_REQ_UNDER4GB].extent = MAX(MIN(early_max_extent, 0x100000000ULL) -
                                     0x100000, 0);

  rs[PAGE_REQ_NONE].start = 0x100000000ULL;
  rs[PAGE_REQ_NONE].extent = (early_max_extent > 0x100000000ULL) ?
    early_max_extent - 0x100000000ULL : 0;

  size_t overheads[3];
  overheads[0] = buddy_calc_overhead(rs[0]);
  overheads[1] = buddy_calc_overhead(rs[1]);
  overheads[2] = buddy_calc_overhead(rs[2]);

  size_t bitmap_sz = overheads[0] + overheads[1] + overheads[2];
  size_t bitmap_sz_pages = round_to_page_size(bitmap_sz) >> get_page_shift();

  for (unsigned i = 0; i < bitmap_sz_pages; ++i) {
    int r = map(MMAP_PMM_BITMAP + i * get_page_size(),
               early_alloc_page(), 1, PAGE_WRITE);
    assert(r == 0 && "Failed to map page");
  }

  int ok;
  ok  = buddy_init(&allocators[PAGE_REQ_UNDER1MB],
                   (uint8_t*) MMAP_PMM_BITMAP,
                   rs[PAGE_REQ_UNDER1MB], 0);
  ok |= buddy_init(&allocators[PAGE_REQ_UNDER4GB],
                   (uint8_t*) (MMAP_PMM_BITMAP + overheads[0]),
                   rs[PAGE_REQ_UNDER4GB], 0);
  ok |= buddy_init(&allocators[PAGE_REQ_NONE],
                   (uint8_t*) (MMAP_PMM_BITMAP + overheads[0] + overheads[1]),
                   rs[PAGE_REQ_NONE], 0);
  if (ok != 0) {
    dbg("buddy_init failed!\n");
    return 1;
  }

  for (unsigned i = 0; i < early_nranges; ++i) {
    if (early_ranges[i].extent == 0)
      continue;

    range_t r = split_range(&early_ranges[i], 0x100000);
    if (r.extent > 0)
      buddy_free_range(&allocators[PAGE_REQ_UNDER1MB], r);

    r = split_range(&early_ranges[i], 0x100000000ULL);
    if (r.extent > 0)
      buddy_free_range(&allocators[PAGE_REQ_UNDER4GB], r);

    if (early_ranges[i].extent > 0)
      buddy_free_range(&allocators[PAGE_REQ_NONE], early_ranges[i]);
  }

  pmm_init_stage = PMM_INIT_FULL;

  return 0;
}
