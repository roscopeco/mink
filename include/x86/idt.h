/* idt.h - Interrupt descriptor table for Mink.
 *
 * Portions based on code from http://www.osdever.net/bkerndev/Docs/idt.htm
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#ifndef __MINK_IDT_H
#define __MINK_IDT_H

// Typedef for ISR/IRQ handler function pointers
typedef int (*isr_func)(isr_regs_t*);

void idt_install(void);
void idt_set_gate(unsigned char num, unsigned long base, unsigned short sel, unsigned char flags);

#endif
