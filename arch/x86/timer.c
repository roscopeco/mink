/* timer.h - Core system PIT handling for Mink. 
 *
 * Portions based on code from http://www.osdever.net/bkerndev/Docs/pit.htm
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#include "hal.h"
#include "sys.h"
#include "utils.h"

/* This will keep track of how many ticks that the system
*  has been running for */
static unsigned long long timer_ticks = 0;

int timer_handler(__attribute__((unused)) isr_regs_t *r) {
  /* Increment our 'tick count' */
  timer_ticks++;
  if (((int)timer_ticks) % 100 == 0) {
    printk(".");
  }
  return 0;
}

void set_kernel_frequency(int hz) {
  int divisor = 1193180 / hz;       /* Calculate our divisor */
  outportb(0x43, 0x36);             /* Set our command byte 0x36 */
  outportb(0x40, divisor & 0xFF);   /* Set low byte of divisor */
  outportb(0x40, divisor >> 8);     /* Set high byte of divisor */
}

/* required by hal.h */
unsigned long long uptime_ticks() {
  return timer_ticks;
}

static int timer_init() {
  // set up for 100hz heartbeat
  set_kernel_frequency(X86_KERNEL_FREQ);
  
  // Set up the timer handler
  install_isr(IRQ_ISR(0x00), timer_handler);
  
  return 1;
}


static feature_prereq_t hard_prereqs[] = { {"x86/idt",NULL}, {"x86/isr",NULL}, {"x86/irq",NULL}, {NULL,NULL} };
static feature_prereq_t prereqs[] = { {"debugger",NULL}, {NULL,NULL} };
static feature_t x MINK_FEATURE = {
  .name = "x86/pit",
  .required = hard_prereqs,
  .load_after = prereqs,
  .init = &timer_init,
};

