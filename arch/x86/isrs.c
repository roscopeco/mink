/* isrs.c - CPU exception ISR Handlers for Mink.
 *
 * Portions based on code from http://www.osdever.net/bkerndev/Docs/isrs.htm
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
#include "hal.h"
#include "sys.h"
#include "x86/idt.h"

/* These are function prototypes for all of the exception
*  handlers: The first 32 entries in the IDT are reserved
*  by Intel for CPU exceptions. */
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

/* Set up the IDT gates for the first 32 interrupts.
 * The IRQs get remapped from 32 to 47. Anything else
 * will just be unhandled for now...
 *
 * Flags 0x8E mean: entry present, in ring 0.
 */
static int isrs_init() {
  idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
  idt_set_gate(1, (unsigned)isr1, 0x08, 0x8E);
  idt_set_gate(2, (unsigned)isr2, 0x08, 0x8E);
  idt_set_gate(3, (unsigned)isr3, 0x08, 0x8E);
  idt_set_gate(4, (unsigned)isr4, 0x08, 0x8E);
  idt_set_gate(5, (unsigned)isr5, 0x08, 0x8E);
  idt_set_gate(6, (unsigned)isr6, 0x08, 0x8E);
  idt_set_gate(7, (unsigned)isr7, 0x08, 0x8E);
  idt_set_gate(8, (unsigned)isr8, 0x08, 0x8E);
  idt_set_gate(9, (unsigned)isr9, 0x08, 0x8E);
  idt_set_gate(10, (unsigned)isr10, 0x08, 0x8E);
  idt_set_gate(11, (unsigned)isr11, 0x08, 0x8E);
  idt_set_gate(12, (unsigned)isr12, 0x08, 0x8E);
  idt_set_gate(13, (unsigned)isr13, 0x08, 0x8E);
  idt_set_gate(14, (unsigned)isr14, 0x08, 0x8E);
  idt_set_gate(15, (unsigned)isr15, 0x08, 0x8E);
  idt_set_gate(16, (unsigned)isr16, 0x08, 0x8E);
  idt_set_gate(17, (unsigned)isr17, 0x08, 0x8E);
  idt_set_gate(18, (unsigned)isr18, 0x08, 0x8E);
  idt_set_gate(19, (unsigned)isr19, 0x08, 0x8E);
  idt_set_gate(20, (unsigned)isr20, 0x08, 0x8E);
  idt_set_gate(21, (unsigned)isr21, 0x08, 0x8E);
  idt_set_gate(22, (unsigned)isr22, 0x08, 0x8E);
  idt_set_gate(23, (unsigned)isr23, 0x08, 0x8E);
  idt_set_gate(24, (unsigned)isr24, 0x08, 0x8E);
  idt_set_gate(25, (unsigned)isr25, 0x08, 0x8E);
  idt_set_gate(26, (unsigned)isr26, 0x08, 0x8E);
  idt_set_gate(27, (unsigned)isr27, 0x08, 0x8E);
  idt_set_gate(28, (unsigned)isr28, 0x08, 0x8E);
  idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);
  idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);
  idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);
  
  return 1;
}

/* the default handler uses these... */
const char *exception_messages[] = {
  "Division By Zero",
  "Debug Exception",
  "Non Maskable Interrupt",
  "Breakpoint Exception",
  "Into Detected Overflow Exception",
  "Out Of Bounds Exception",
  "Invalid Opcode Exception",
  "No Coprocessor Exception",
  "Double Fault Exception",
  "Coprocessor Segment Overrun Exception",
  "Bad TSS Exception",
  "Segment Not Present Exception",
  "Stack Fault",
  "General Protection Fault",
  "Page Fault",
  "Unknown Interrupt Exception",
  "Coprocessor Fault",
  "Alignment Check Exception",
  "Machine Check Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception",
  "Reserved Exception"
};

isr_func isr_routines[256] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/* PLATFORM SPECIFIC NOTICE: x86/mem MUST be up before this is called! */
void install_isr(unsigned int intr, int (*handler)(isr_regs_t *r)) {
  if (intr > 0 && intr < 256) {
    isr_routines[intr] = handler;
  }
}

void uninstall_isr(unsigned int intr) {
  if (intr > 0 && intr < 256) {
    isr_routines[intr] = 0;
  }
}

/* Fault handler for exceptions. IRQ's go through a different handler
 * (see irqs.h).
 */
void fault_handler(isr_regs_t *r) {
  int (*handler)(isr_regs_t *r);

  /* Find out if we have a custom handler to run for this interrupt. */
  handler = isr_routines[r->int_no];
  if (handler) {
  	int ret = handler(r);
    if (ret) {
    	// handlers should return 0 on success
    	panic("Handler @ 0x%08x returned 0x%08x (FAILED)", handler, ret);
    }
  } else {
    // default handler
    if (r->int_no < 32) {
      panic("%s: 0x%02x (errno %d)", exception_messages[r->int_no], r->int_no, r->err_code);
    } else {
      panic("Unhandled exception (don't know what; try Google): 0x%02x (errno %d)", r->int_no, r->err_code);
    }
  }     
}

static feature_prereq_t hard_prereqs[] = { {"x86/idt",NULL}, {NULL,NULL} };
static feature_prereq_t prereqs[] = { {"debugger",NULL}, {NULL,NULL} };
static feature_t x MINK_FEATURE = {
  .name = "x86/isr",
  .required = hard_prereqs,
  .load_after = prereqs,
  .init = &isrs_init,
};

