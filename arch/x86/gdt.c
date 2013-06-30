/* gdt.c - x86 GDT Handling for Mink.
 *
 * This is heavily influenced by James Molloy's JMTK.
 * Portions copyright (c)2012 James Molloy.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#include <stddef.h>
#include <stdint.h>
#include "hal.h"
#include "sys.h"
#include "utils.h"

typedef struct tss_entry {
  uint32_t prev_tss;
  uint32_t esp0, ss0, esp1, ss1, esp2, ss2;
  uint32_t cr3, eip, eflags;
  uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
  uint32_t es, cs, ss, ds, fs, gs;
  uint32_t ldt;
  uint16_t trap, iomap_base;
} tss_entry_t;

typedef struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t  base_mid;
  uint8_t  type : 4;
  uint8_t  s    : 1;            /* 's' should always be 1, except for */
  uint8_t  dpl  : 2;            /* the NULL segment. */
  uint8_t  p    : 1;
  uint8_t  limit_high : 4;
  uint8_t  avail: 1;
  uint8_t  l    : 1;
  uint8_t  d    : 1;
  uint8_t  g    : 1;
  uint8_t  base_high;
} gdt_entry_t;

typedef struct gdt_ptr {
  uint16_t limit;               /* Size of the GDT */
  uint32_t base;                /* Start of the GDT */
} __attribute__((packed)) gdt_ptr_t;

static gdt_ptr_t gdt_ptr;
static gdt_entry_t entries[MAX_CORES+5];
static tss_entry_t tss_entries[MAX_CORES];

unsigned num_gdt_entries, num_tss_entries;

static 
void set_gdt_entry(gdt_entry_t *e, uint32_t base, uint32_t limit,
                   uint8_t type, uint8_t s, uint8_t dpl, uint8_t p, uint8_t l,
                   uint8_t d, uint8_t g) {
  e->limit_low  = limit & 0xFFFF;
  e->base_low   = base & 0xFFFF;
  e->base_mid   = (base >> 16) & 0xFF;
  e->type       = type & 0xF;
  e->s          = s & 0x1;
  e->dpl        = dpl & 0x3;
  e->p          = p & 0x1;
  e->limit_high = (limit >> 16) & 0xF;
  e->avail      = 0;
  e->l          = l & 0x1;
  e->d          = d & 0x1;
  e->g          = g & 0x1;
  e->base_high  = (base >> 24) & 0xFF;
}

static void set_tss_entry(tss_entry_t *e) {
  memset((uint8_t*)e, 0, sizeof(tss_entry_t));
  e->ss0 = e->ss = e->ds = e->es = e->fs = e->gs = 0x10;
  e->cs = 0x08;
}

#define TY_CODE 8

/* Applies to code segments */
#define TY_CONFORMING 4
#define TY_READABLE 2

/* Applies to data segments. */
#define TY_DATA_EXPAND_DIRECTION 4
#define TY_DATA_WRITABLE 2

/* Applies to both; set by the CPU. */
#define TY_ACCESSED 1

static int gdt_init() {
  /*                         Base Limit Type                 S  Dpl P  L  D  G*/
  set_gdt_entry(&entries[0], 0,  0xFFF0, 0,                  0, 0,  0, 0, 0, 0);
  set_gdt_entry(&entries[1], 0,   ~0U,  TY_CODE|TY_READABLE, 1, 0,  1, 0, 1, 1);
  set_gdt_entry(&entries[2], 0,   ~0U,  TY_DATA_WRITABLE,    1, 0,  1, 0, 1, 1);
  set_gdt_entry(&entries[3], 0,   ~0U,  TY_CODE|TY_READABLE, 1, 3,  1, 0, 1, 1);
  set_gdt_entry(&entries[4], 0,   ~0U,  TY_DATA_WRITABLE,    1, 3,  1, 0, 1, 1);

  int num_processors = get_num_cpucores();
  for (int i = 0; i < num_processors; ++i) {
    set_tss_entry(&tss_entries[i]);
    set_gdt_entry(&entries[i+5], (uint32_t)&tss_entries[i],
                                      /* Type                S  Dpl P  L  D  G*/
                  sizeof(tss_entry_t)-1, TY_CODE|TY_ACCESSED,0, 3,  1, 0, 0, 1);
  }

  num_gdt_entries = num_processors + 4;
  num_tss_entries = num_processors;
  
  gdt_ptr.base = (uint32_t)&entries[0];
  gdt_ptr.limit = sizeof(gdt_entry_t) * num_gdt_entries - 1;

  __asm volatile("lgdt %0;"
                 "mov  $0x10, %%ax;"
                 "mov  %%ax, %%ds;"
                 "mov  %%ax, %%es;"
                 "mov  %%ax, %%fs;"
                 "mov  %%ax, %%gs;"
                 "ljmp $0x08, $1f;"
                 "1:" : : "m" (gdt_ptr) : "eax");
                
  return 1;
}

static feature_prereq_t prereqs[] = { {"debugger",NULL}, {NULL,NULL} };
static feature_t x MINK_FEATURE = {
  .name = "x86/gdt",
  .required = NULL,
  .load_after = prereqs,
  .init = &gdt_init,
};
