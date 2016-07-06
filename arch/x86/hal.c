/* hal.c - x86-specific HAL implementations for Mink.
 *
 * Copyright (c)2013-2016 Ross Bamford. See LICENSE for details.
 */
#include <stdint.h>

#include "hal.h"
#include "sys.h"
#include "x86/vgaterm.h"
#include "utils.h"
#include "elf.h"

elf_t kernel_elf;

void enable_interrupts() {
  __asm__ volatile("sti");
}

void disable_interrupts() {
  __asm__ volatile("cli");
}

int get_interrupt_state() {
  uint32_t eflags;
  __asm__ volatile("pushf; pop %0" : "=r" (eflags));
  return eflags & 0x200;
}

// TODO implement this properly!!
int get_num_cpucores() {
  return 1;
}

noreturn void idle() {
  for (;;) {
    __asm__ volatile("hlt");
  }
}

noreturn void die() {
  disable_interrupts();
  idle();
}

void print_stack_trace() {
  uint32_t *ebp, *eip;
  __asm__ volatile ("mov %%ebp, %0" : "=r" (ebp));
  while (ebp) {
    eip = ebp+1;
    printk ("   [0x%x] %s\n", *eip, elf_lookup_symbol (*eip, &kernel_elf));
    ebp = (uint32_t*) *ebp;
  }
}

elf_t* get_kernel_elf() {
  return &kernel_elf;
}

