/* gdt.c - x86 GDT Handling for Mink.
 *
 * Copyright (c)2013-2018 Ross Bamford. See LICENSE for details.
 */

#include <stddef.h>
#include "x86/gdt.h"
#include "hal.h"
#include "sys.h"
#include "utils.h"

static gdt_ptr_t gdt_ptr;
static gdt_entry_t entries[MAX_CORES+5];
static tss_entry_t tss_entries[MAX_CORES];

unsigned num_gdt_entries, num_tss_entries;

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

static void init_tss_entry(tss_entry_t *e) {
  memset((uint8_t*)e, 0, sizeof(tss_entry_t));
  e->ss0 = e->ss = e->ds = e->es = e->fs = e->gs = 0x10;
  e->cs = 0x08;
}

void update_tss_entry(uint16_t cpu_core, uint32_t ss0, uint32_t esp0) {
  tss_entries[cpu_core].ss0 = ss0;
  tss_entries[cpu_core].esp0 = esp0;
}

static int gdt_init() {
  /*                         Base Limit Type                 S  Dpl P  L  D  G*/
  set_gdt_entry(&entries[0], 0,  0xFFF0, 0,                  0, 0,  0, 0, 0, 0);
  set_gdt_entry(&entries[1], 0,   ~0U,  TY_CODE|TY_READABLE, 1, 0,  1, 0, 1, 1);
  set_gdt_entry(&entries[2], 0,   ~0U,  TY_DATA_WRITABLE,    1, 0,  1, 0, 1, 1);
  set_gdt_entry(&entries[3], 0,   ~0U,  TY_CODE|TY_READABLE, 1, 3,  1, 0, 1, 1);
  set_gdt_entry(&entries[4], 0,   ~0U,  TY_DATA_WRITABLE,    1, 3,  1, 0, 1, 1);

  int num_processors = get_num_cpucores();
  for (int i = 0; i < num_processors; ++i) {
    init_tss_entry(&tss_entries[i]);
    set_gdt_entry(&entries[i+5], (uint32_t)&tss_entries[i],
                                      /* Type                S  Dpl P  L  D  G*/
                  sizeof(tss_entry_t), TY_CODE|TY_ACCESSED,  0, 3,  1, 0, 0, 1);
  }

  num_gdt_entries = num_processors + 5;
  num_tss_entries = num_processors;
  
  gdt_ptr.base = (uint32_t)&entries[0];
  gdt_ptr.limit = sizeof(gdt_entry_t) * num_gdt_entries - 1;

  __asm volatile("lgdt %0;"
                 "mov  $0x10, %%ax;"
                 "mov  %%ax, %%ds;"
                 "mov  %%ax, %%es;"
                 "mov  %%ax, %%fs;"
                 "mov  %%ax, %%gs;"
                 "ljmp $0x08, $flushtss;"
                 "flushtss:"
                 "mov  $0x28, %%ax;"
                 "ltr  %%ax;"
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
