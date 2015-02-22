/* tick.c - Core timer tick for Mink kernel.
 *
 * This is called from the (architecture-dependent) timer interrupt
 * handler to actually handle a kernel clock tick.
 *
 * All code here should be arch-agnostic. Anything required by a
 * specific arch should be done in the ISR itself either before
 * or after calling this...
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#include "utils.h"

/* This will keep track of how many ticks that the system
*  has been running for */
static unsigned long long jiffies = 0;

/* required by hal.h */
unsigned long long uptime_jiffies() {
  return jiffies;
}

void kernel_tick() {
  /* Increment our 'tick count' */
  jiffies++;
  if (((int)jiffies) % 100 == 0) {
    printk(".");
  }
}
