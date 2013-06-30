/* locking.c - Spinlocks and semaphores.
 *
 * This file is taken directly from James Malloy's JMTK tutorial
 * code. Copyright (c)2012 James Molloy.
 */
#include "hal.h"

void spinlock_init(spinlock_t *lock) {
  lock->val = 0;
  lock->interrupts = 0;
}

void spinlock_acquire(spinlock_t *lock) {
  int interrupts = get_interrupt_state();

  disable_interrupts();
  while (__sync_bool_compare_and_swap(&lock->val, 0, 1) == 0)
    ;

  lock->interrupts = interrupts;
}

void spinlock_release(spinlock_t *lock) {
  while (__sync_bool_compare_and_swap(&lock->val, 1, 0) == 0)
    ;
    
  if (lock->interrupts) {
    enable_interrupts();
  }
}

