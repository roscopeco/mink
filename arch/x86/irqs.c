/* irqs.c - IRQ handling for Mink.
 *
 * Portions based on code from http://www.osdever.net/bkerndev/Docs/irqs.htm
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
#include "hal.h" 
#include "sys.h"
#include "utils.h"
#include "x86/idt.h"

extern void* isr_routines[256];

/* ISR stubs that will call back to irq_handler (see irq_stubs.s) */
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

/* Remap IRQ's 0-7 to start at IDT 32, so we have a nice
 * clean contiguous run (IRQ's 0-15 run from IDT 32 to 47.
 */
static void irq_remap(void)
{
  outportb(0x20, 0x11);
  outportb(0xA0, 0x11);
  outportb(0x21, 0x20);
  outportb(0xA1, 0x28);
  outportb(0x21, 0x04);
  outportb(0xA1, 0x02);
  outportb(0x21, 0x01);
  outportb(0xA1, 0x01);
  outportb(0x21, 0x0);
  outportb(0xA1, 0x0);
}

static int irqs_init() {
  irq_remap();

  idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
  idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
  idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
  idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
  idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
  idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
  idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
  idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);
  idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
  idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
  idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
  idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
  idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
  idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
  idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
  idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);

  return 1;
}

/* Each of the IRQ ISRs point to this function, rather than
*  the 'fault_handler' in 'isrs.c'. The IRQ Controllers need
*  to be told when we're done servicing them, so we need
*  to send them an "End of Interrupt" command (0x20). There
*  are two 8259 chips: The first exists at 0x20, the second
*  exists at 0xA0. If the second controller (an IRQ from 8 to
*  15) gets an interrupt, we need to acknowledge the
*  interrupt at BOTH controllers, otherwise, we only send
*  an EOI command to the first controller. If we don't send
*  an EOI, we won't get any more IRQs */
void irq_handler(isr_regs_t *r) {
  void (*handler)(isr_regs_t *r);

	/* Find out if we have a custom handler to run for this interrupt.
   * Unlike with exception handlers, if we don't have a handler we're
   * just going to ignore the IRQ (it may be a device we don't have
   * a driver for or something) instead of panicking. */
  handler = isr_routines[r->int_no];
  if (handler) {
    handler(r);
  } else {
  	printk("Unhandled IRQ %d\n", ISR_IRQ(r->int_no));
  }
  
  /* Reset slave controller if needs be... */
  if (r->int_no >= 40) {
    outportb(0xA0, 0x20);
  }

  /* ... and reset the master. */
  outportb(0x20, 0x20);
}

static feature_prereq_t hard_prereqs[] = { {"x86/idt",NULL}, {"x86/isr",NULL}, {NULL,NULL} };
static feature_prereq_t prereqs[] = { {"debugger",NULL}, {NULL,NULL} };
static feature_t x MINK_FEATURE = {
  .name = "x86/irq",
  .required = hard_prereqs,
  .load_after = prereqs,
  .init = &irqs_init,
};

