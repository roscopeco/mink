/* x86_64/hal.h - x86_64 specific HAL for Mink.
 *
 * This file defines certain structures that are required to be defined in an
 * architecture-specific way.
 *
 * Copyright (c)2013-2016 Ross Bamford. See LICENSE for details.
 */

#ifndef __MINK_X86_64_HAL_H
#define __MINK_X86_64_HAL_H

#define X86_KERNEL_FREQ 100 /* hz */

/* This defines what the stack looks like when an ISR is called. */
typedef struct isr_regs {
  unsigned int gs, fs, es, ds;      /* pushed the segs last */
  unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;  /* pushed by 'pusha' */
  unsigned int int_no, err_code;    /* our 'push byte #' and ecodes do this */
  unsigned int eip, cs, eflags, useresp, ss;   /* pushed by the processor automatically */ 
} isr_regs_t;

/* This macro can be used when registering ISRs for IRQs. You give it the IRQ
 * number, and it gives you back the actual ISR number it maps to.
 * 
 * e.g.:
 *
 *    install_isr(IRQ_ISR(3));
 */
#define IRQ_ISR(x) (x + 0x20)

/* This macro can be used to convert an ISR number into it's
 * corresponding IRQ number (or -1 if not an IRQ).
 *
 * -1U seems a bit counter-intuitive, but it stops a GCC warning
 * and doesn't make any difference, so it is what it is...
 */
#define ISR_IRQ(x) ((x > 31U && x < 48U) ? (x - 0x20) : -1U)

#define THREAD_STACK_SZ 0x2000  /* 8KB of kernel stack. */

#define X86_PRESENT 0x1
#define X86_WRITE   0x2
#define X86_USER    0x4
#define X86_EXECUTE 0x200
#define X86_COW     0x400

typedef struct address_space {
  uint32_t *directory;
  spinlock_t lock;
} address_space_t;

static inline unsigned get_page_size() {
  return 4096;
}

static inline unsigned get_page_shift() {
  return 12;
}

static inline unsigned get_page_mask() {
  return 0xFFF;
}

static inline uintptr_t round_to_page_size(uintptr_t x) {
  if ((x & 0xFFF) != 0)
    return ((x >> 12) + 1) << 12;
  else
    return x;
}

static inline void abort() {
  for(;;);
}

struct jmp_buf_impl {
  uint32_t esp, ebp, eip, ebx, esi, edi, eflags;
};

typedef struct jmp_buf_impl jmp_buf[1];

static inline void jmp_buf_set_stack(jmp_buf buf, uintptr_t stack) {
  buf[0].esp = stack;
}

static inline void jmp_buf_to_regs(isr_regs_t *r, jmp_buf buf) {
  r->esp = buf[0].esp;
  r->ebp = buf[0].ebp;
  r->eip = buf[0].eip;
  r->ebx = buf[0].ebx;
  r->esi = buf[0].edi;
  r->eflags = buf[0].eflags;
}

#define abort() (void)0

#define CR0_PG  (1U<<31)  /* Paging enable */
#define CR0_WP  (1U<<16)  /* Write-protect - allow page faults in kernel mode */

/* All these single instructions are definied here in the header
 * and just inlined wherever they're used if possible...
 */
static inline void outportb(uint16_t port, uint8_t value) {
  __asm__ volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

static inline void outportw(uint16_t port, uint16_t value) {
  __asm__ volatile ("outw %1, %0" : : "dN" (port), "a" (value));
}

static inline void outportl(uint16_t port, uint32_t value) {
  __asm__ volatile ("outl %1, %0" : : "dN" (port), "a" (value));
}

static inline uint8_t inportb(uint16_t port) {
  uint8_t ret;
  __asm__ volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

static inline uint16_t inportw(uint16_t port) {
  uint16_t ret;
  __asm__ volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

static inline uint32_t inportl(uint16_t port) {
  uint32_t ret;
  __asm__ volatile ("inl %1, %0" : "=a" (ret) : "dN" (port));
  return ret;
}

static inline uint32_t read_cr0() {
  uint32_t ret;
  __asm__ volatile("mov %%cr0, %0" : "=r" (ret));
  return ret;
}
static inline uint32_t read_cr2() {
  uint32_t ret;
  __asm__ volatile("mov %%cr2, %0" : "=r" (ret));
  return ret;
}
static inline uint32_t read_cr3() {
  uint32_t ret;
  __asm__ volatile("mov %%cr3, %0" : "=r" (ret));
  return ret;
}

static inline void write_cr0(uint32_t val) {
  __asm__ volatile("mov %0, %%cr0" : : "r" (val));
}
static inline void write_cr2(uint32_t val) {
  __asm__ volatile("mov %0, %%cr2" : : "r" (val));
}
static inline void write_cr3(uint32_t val) {
  __asm__ volatile("mov %0, %%cr3" : : "r" (val));
}

#endif
