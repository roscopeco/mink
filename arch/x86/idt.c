/* idt.c - Interrupt descriptor table for Mink.
 *
 * Portions based on code from http://www.osdever.net/bkerndev/Docs/idt.htm
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#include "hal.h"
#include "sys.h"

/* Defines an IDT entry */
struct idt_entry
{
    unsigned short base_lo;
    unsigned short sel;        /* Our kernel segment goes here! */
    unsigned char always0;     /* This will ALWAYS be set to 0! */
    unsigned char flags;       /* Set using the above table! */
    unsigned short base_hi;
} __attribute__((packed));

struct idt_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

/* Declare an IDT of 256 entries. Although we only use the
 * first 32 entries for now, the rest exists as a bit
 * to make sure any others generate an "Unhandled Interrupt".
 */
struct idt_entry idt[256];
struct idt_ptr idtp;

/* Set an entry in the IDT. */
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags) {
  idt[num].base_lo = (base & 0xFFFF);
  idt[num].base_hi = (base >> 16) & 0xFFFF;
  idt[num].sel = sel;
  idt[num].always0 = 0;
  idt[num].flags = flags;
}

/* Installs the IDT */
static int idt_init() {
  /* Sets the special IDT pointer up, just like in 'gdt.c' */
  idtp.limit = (sizeof (struct idt_entry) * 256) - 1;
  idtp.base = (unsigned int)&idt;

  /* Clear out the entire IDT, initializing it to zeros */
  memset(&idt, 0, sizeof(struct idt_entry) * 256);

  /* Add any new ISRs to the IDT here using idt_set_gate */

  /* Points the processor's internal register to the new IDT */
  __asm__ volatile("lidt %0" :: "m" (idtp));
  
  return 1;
}

static feature_t x MINK_FEATURE = {
  .name = "x86/idt",
  .required = NULL, 
  .load_after = NULL,
  .init = &idt_init,
};

