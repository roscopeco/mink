/* hal.h - Hardware Abstraction Layer for Mink Kernel.
 *
 * Each target architecture will have it's own implementation of the functions
 * defined herein.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
 
#ifndef _MINK_HAL_H
#define _MINK_HAL_H

#include <stdint.h>
#include <stddef.h>
#ifndef __CPLUSPLUS
#include <stdbool.h>
#endif
#include "elf.h"

#if defined(__GNUC__)
#define __MINK_NORETURN __attribute__((noreturn))
#else
// TODO Define for other compilers if needed?
#define __MINK_NORETURN
#endif

typedef struct spinlock {
  volatile unsigned val;
  volatile unsigned interrupts;
} spinlock_t;

/* Each architecture must have an arch-specific HAL header that defines:
 * 
 *    The 'isr_regs_t' struct type that passes registers etc. to an ISR.
 *    The 'address_space_t' struct type that defines an address space.
 */
#if defined(X86)
#include "x86/hal.h"
#else
#error Unsupported architecture
#endif

#define MAX_CORES 256

/****************************************************************
 * CPU & INTERRUPTS
 */
/**
 * Disable CPU interrupts.
 */
void disable_interrupts();

/**
 * Enable CPU interrupts.
 */
void enable_interrupts();

/**
 * Get current interrupt state (0 = disabled).
 */
int get_interrupt_state();

/**
 * Install an interrupt service routine on the specified interrupt.
 * Currently, only one handler per interrupt is supported.
 */
void install_isr(unsigned int num, int (*handler)(isr_regs_t *r));

/**
 * Remove the interrupt service routine on the specified interrupt.
 */
void uninstall_isr(unsigned int num);

/**
 * Determine the number of CPU cores available on this system.
 */
int get_num_cpucores();

/**
 * Idle CPU (never returns) 
 */
void idle() __MINK_NORETURN;

/**
 * Halt machine (disable interrupts and idle; never returns).
 */
void die() __MINK_NORETURN;


/****************************************************************
 * IO PORTS
 * 
 * Possibly not applicable to all platforms, but will only ever be used by 
 * platform-specific code. 
 *
 * These will probably be removed from here and placed in platform-specific
 * headers...
 */
/**
 * Read a byte from the specified IO port.
 */
unsigned char inportb (unsigned short _port);

/**
 * Write a byte to the specified IO port.
 */
void outportb (unsigned short _port, unsigned char _data);


/****************************************************************
 * MEMORY MANAGEMENT
 */
#define PAGE_WRITE   1 /* Page is writable */
#define PAGE_EXECUTE 2 /* Page is executable */
#define PAGE_USER    4 /* Page is useable by user mode code (else kernel only) */
#define PAGE_COW     8 /* Page is marked copy-on-write. It must be copied if
                          written to. */

#define PAGE_REQ_NONE     0 /* No requirements on page location */
#define PAGE_REQ_UNDER1MB 1 /* Require that the returned page be < 0x100000 */
#define PAGE_REQ_UNDER4GB 2 /* Require that the returned page be < 0x10000000 */

/* Returns the (default) page size in bytes. Not all pages may be this size
   (large pages etc.) */
unsigned get_page_size();

/* Rounds an address up so that it is page-aligned. */
uintptr_t round_to_page_size(uintptr_t x);

/* Allocate a physical page of the size returned by get_page_size().
   This should only be used between calling init_physical_memory_early() and
   init_physical_memory(). */
uint64_t early_alloc_page();

/* Allocate a physical page of the size returned by get_page_size(), returning
   the address of the page in the physical address space. Returns ~0ULL on
   failure.

   'req' is one of the 'PAGE_REQ_*' flags, indicating a requirement on the
   address of the returned page. */
uint64_t alloc_page(int req);
/* Mark a physical page as free. Returns -1 on failure. */
int free_page(uint64_t page);

uint64_t alloc_pages(int req, size_t num);
int free_pages(uint64_t pages, size_t num);

/* Creates a new address space based on the current one and stores it in
   'dest'. If 'make_cow' is nonzero, all pages marked WRITE are modified so
   that they are copy-on-write. */
int clone_address_space(address_space_t *dest, int make_cow);

/* Switches address space. Returns -1 on failure. */
int switch_address_space(address_space_t *dest);

/* Returns the current address space. */
address_space_t *get_current_address_space();

/* Maps 'num_pages' * get_page_size() bytes from 'p' in the physical address
   space to 'v' in the current virtual address space.

   Returns zero on success or -1 on failure. */
int map(uintptr_t v, uint64_t p, int num_pages,
        unsigned flags);
/* Unmaps 'num_pages' * get_page_size() bytes from 'v' in the current virtual address
   space. Returns zero on success or -1 on failure. */
int unmap(uintptr_t v, int num_pages);

/* If 'v' has a V->P mapping associated with it, return 'v'. Else return
   the next page (multiple of get_page_size()) which has a mapping associated
   with it. */
uintptr_t iterate_mappings(uintptr_t v);

/* If 'v' is mapped, return the physical page it is mapped to
   and fill 'flags' with the mapping flags. Else return ~0ULL. */
uint64_t get_mapping(uintptr_t v, unsigned *flags);

/* Return 1 if 'v' is mapped, else 0, or -1 if not implemented. */
int is_mapped(uintptr_t v);

/* A range of memory, with a start and a size. */
typedef struct range {
  uint64_t start;
  uint64_t extent;
} range_t;

/* Initialise the virtual memory manager.
   
   Returns 0 on success or -1 on failure. */
int init_virtual_memory();

/* Initialise the physical memory manager (stage 1), passing in a set
   of ranges and the maximum extent of physical memory
   (highest address + 1).

   The set of ranges will be copied, not mutated. */
int init_physical_memory_early(range_t *ranges, unsigned nranges,
                               uint64_t extent);

/* Initialise the physical memory manager (stage 2). This should be 
   done after the virtual memory manager is set up. */
int init_physical_memory();

/* Initialise the copy-on-write page reference counts. */
int init_cow_refcnts(range_t *ranges, unsigned nranges);

/* Increment the reference count of a copy-on-write page. */
void cow_refcnt_inc(uint64_t p);

/* Decrement the reference count of a copy-on-write page. */
void cow_refcnt_dec(uint64_t p);

/* Return the reference count of a copy-on-write page. */
unsigned cow_refcnt(uint64_t p);

/* Handle a page fault potentially caused by a copy-on-write access.

   'addr' is the address of the fault. 'error_code' is implementation 
   defined. Returns true if the fault was copy-on-write related and was
   handled, false if it still needs handling. */
bool cow_handle_page_fault(uintptr_t addr, uintptr_t error_code);


/********************************************************************************
 * Threading/Locking
 *******************************************************************************/

#define SPINLOCK_RELEASED {.val=0, .interrupts=0};
#define SPINLOCK_ACQUIRED {.val=1, .interrupts=0};

/* Initialise a spinlock to the released state. */
void spinlock_init(spinlock_t *lock);
/* Returns a new, initialised spinlock. */
spinlock_t *spinlock_new();
/* Acquire 'lock', blocking until it is available. */
void spinlock_acquire(spinlock_t *lock);
/* Release 'lock'. Nonblocking. */
void spinlock_release(spinlock_t *lock);


/****************************************************************
 * KERNEL FEATURES
 *
 * Features are statically-linked, dynamically initialised modules built 
 * in to the kernel. They provide basic functionality such as console IO,
 * interrupt handling and so forth - most of which are architecture 
 * dependent, and many of which don't apply to all architectures. 
 * 
 * Handling them in this way means that the kernel is more maintainable,
 * and it's easy to add new modules for different architectures. It also
 * means that the architecture-agnostic code in kmain doesn't need any
 * architecture-specific calls, and doesn't need to make a ton of calls
 * to architecture specific methods, which would just be stubbed on other
 * architectures.
 *
 * Obviously we could just have a catch-all 'init_arch' in the HAL, but
 * that would mean we'd just be handing over control. It might also be
 * less portable...
 *
 * This is actually based on James Malloy's code from his JMTK. He calls
 * features 'modules'. I avoided that to avoid confusion with loadable
 * modules.
 */

/* A feature prerequisite. A client should only fill in the 'name' member -
   the 'feature' member is filled in by main.c. */
typedef struct feature_prereq {
  const char *name;
  struct feature *feature;
} feature_prereq_t;

/* Structure defining a function to be run either on startup or shutdown. */
typedef struct feature {
  /* Members that should be initialised statically. */
  const char *name;             /* A unique identifier for this function.*/
  feature_prereq_t *required;   /* Either NULL or a NULL-terminated list of
                                   module names that are prerequisites of this. */
  feature_prereq_t *load_after; /* Either NULL or a NULL-terminated list of 
                                   module names that are not hard prerequisites
                                   but if available then this module must be loaded
                                   after them */
  int (*init)(void);            /* The startup function to run. Returns >0 on success */

  uintptr_t state;              /* used internally - don't play with this! */
  uintptr_t reserved2[2];
} feature_t;

/* Mark a feature_t as 'MINK_FEATURE' to run it during kernel init e.g.:  
     static feature_t x MINK_FEATURE = {...}; */
#define MINK_FEATURE __attribute__((__section__("features"),unused,used))


/****************************************************************
 * CONSOLE IO (Well, just O at the moment...)
 */
enum console_color {
	COLOR_BLACK = 0,
	COLOR_BLUE = 1,
	COLOR_GREEN = 2,
	COLOR_CYAN = 3,
	COLOR_RED = 4,
	COLOR_MAGENTA = 5,
	COLOR_BROWN = 6,
	COLOR_LIGHT_GREY = 7,
	COLOR_DARK_GREY = 8,
	COLOR_LIGHT_BLUE = 9,
	COLOR_LIGHT_GREEN = 10,
	COLOR_LIGHT_CYAN = 11,
	COLOR_LIGHT_RED = 12,
	COLOR_LIGHT_MAGENTA = 13,
	COLOR_LIGHT_BROWN = 14,
	COLOR_WHITE = 15,
};

#define make_color(fg,bg) (fg | bg << 4)

/**
 * Defines a console.
 */
typedef struct console {
	void (*writestring)(const char *);
	void (*setcolor)(uint8_t color);
	uint8_t (*getcolor)();
} console_t;
 
/**
 * Register a console handler. Only four consoles are supported -
 * if the maximum number are already registered, this function
 * will return false.
 */
bool console_register(console_t con);

/**
 * Write the given string to all consoles.
 */
void console_writestring(const char *str);

/**
 * Change the color of the console output.
 * This is optional - if unsupported on your architecture, you 
 * should just ignore this call.
 */
void console_setcolor(uint8_t color);

/**
 * Get the current console output color. If color change is 
 * unsupported, you should just return 0.
 *
 * Note that this function will always return the color of the
 * first console!
 */
uint8_t console_getcolor();

/****************************************************************
 * MISC
 */

/**
 * Get the elf_t (See elf.h) structure for the kernel binary,
 * or NULL if unavailable.
 */
elf_t get_kernel_elf();

/**
 * Print a stack trace.
 */
void print_stack_trace();

/**
 * Set the frequency of the core kernel timer. This is an optional operation -
 * architectures are free to ignore it entirely, or take it as a 'hint',
 * or whatever...
 */
void set_kernel_frequency(int hz);

/**
 * Get system uptime in terms of core timer ticks.
 * Since the kernel frequency is architecture dependent (and maybe even 
 * configurable depending on your platform, and at build-time) this doesn't 
 * really give you anything useful, but there you go...
 */
unsigned long long uptime_jiffies();

#endif
