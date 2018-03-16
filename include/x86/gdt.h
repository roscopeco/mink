/* gdt.h - x86 GDT interface for Mink.
 *
 * Copyright (c)2013-2018 Ross Bamford. See LICENSE for details.
 */

#ifndef __MINK_X86_GDT_H_
#define __MINK_X86_GDT_H_

#include <stdint.h>

/* Applies to code segments */
#define TY_CODE 8
#define TY_CONFORMING 4
#define TY_READABLE 2

/* Applies to data segments. */
#define TY_DATA_EXPAND_DIRECTION 4
#define TY_DATA_WRITABLE 2

/* Applies to both; set by the CPU. */
#define TY_ACCESSED 1

/* Type field for TSS */
#define TY_TSS 9

typedef struct tss_entry {
  uint32_t prev_tss;
  uint32_t esp0, ss0, esp1, ss1, esp2, ss2;
  uint32_t cr3, eip, eflags;
  uint32_t eax, ecx, edx, ebx, esp, ebp, esi, edi;
  uint32_t es, cs, ss, ds, fs, gs;
  uint32_t ldt;
  uint16_t trap, iomap_base;
} __attribute__((packed)) tss_entry_t;

typedef struct gdt_entry {
  uint16_t limit_low;       /* low 16 bits of limit */
  uint16_t base_low;        /* low 16 bits of base */
  uint8_t  base_mid;        /* low byte of high word of base */
  uint8_t  type : 4;        /* descriptor type */
  uint8_t  s    : 1;        /* 0 = system descriptor, 1 = code/data descriptor */
  uint8_t  dpl  : 2;        /* descriptor privilege level */
  uint8_t  p    : 1;        /* 0 = not present, 1 = present */
  uint8_t  limit_high : 4;  /* high 4 bits of limit */
  uint8_t  avail: 1;        /* Not used - reserved for OS (not using yet) */
  uint8_t  l    : 1;        /* 64-bit code segment flag - always 0 on this arch (See Intel 3A, 3-12) */
  uint8_t  d    : 1;        /* default size/upper bound - always 1 on this arch (See Intel 3A, 3-11) */
  uint8_t  g    : 1;        /* granularity - always 1 on this arch (4KB granularity) */
  uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt_ptr {
  uint16_t limit;               /* Size of the GDT */
  uint32_t base;                /* Start of the GDT */
} __attribute__((packed)) gdt_ptr_t;

void set_gdt_entry(gdt_entry_t *e, uint32_t base, uint32_t limit,
                   uint8_t type, uint8_t s, uint8_t dpl, uint8_t p, uint8_t l,
                   uint8_t d, uint8_t g);

void update_tss_entry(uint16_t cpu_core, uint32_t ss0, uint32_t esp0);

#endif /* INCLUDE_X86_GDT_H_ */
