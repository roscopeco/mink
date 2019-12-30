/* vmm.c - Virtual Memory Manager for Mink kernel.
 *
 * Portions copyright (c)2012 James Molloy.
 *
 * The memory management (both pmm and vmm) are exposed to the kernel
 * via the x86/mem feature (defined in x86/mem.c).
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */
#include "hal.h"
#include "mmap.h"
#include "sys.h"
#include "utils.h"

#if defined(KDEBUG_ENABLED) && defined(KDEBUG_VMM)
# define dbg(...) printk("vmm: " __VA_ARGS__)
#else
# define dbg(...)
#endif

/**
   We defined the struct ``address_space_t`` in our platform-specific HAL header, to just be a pointer to a 32-bit integer (for the page directory) and a spinlock to serialize accesses to the page tables. { */

static address_space_t *current = NULL;

static spinlock_t global_vmm_lock = SPINLOCK_RELEASED;

/**
   These helper functions just map between the x86-specific page flag values and the architecture-independent flag values defined in hal.h. { */

static int from_x86_flags(int flags) {
  int f = 0;
  if (flags & X86_WRITE)   f |= PAGE_WRITE;
  if (flags & X86_EXECUTE) f |= PAGE_EXECUTE;
  if (flags & X86_USER)    f |= PAGE_USER;
  if (flags & X86_COW)     f |= PAGE_COW;
  return f;
}
static int to_x86_flags(int flags) {
  int f = 0;
  if (flags & PAGE_WRITE)   f |= X86_WRITE;
  if (flags & PAGE_USER)    f |= X86_USER;
  if (flags & PAGE_EXECUTE) f |= X86_EXECUTE;
  if (flags & PAGE_COW)     f |= X86_COW;
  return f;
}

address_space_t *get_current_address_space() {
  return current;
}

/**
   Now we can write the code to inform the CPU about a page directory. To do this, we write the (**physical**) address of the directory to the ``%cr3`` register, along with the access flags PRESENT and WRITEable. { */

int switch_address_space(address_space_t *dest) {
  write_cr3((uintptr_t)dest->directory | X86_PRESENT | X86_WRITE);
  return 0;
}

/**
"The recursive page directory trick"
====================================

Notice I mentioned above that the address of the page directory that we write is a *physical* address. This kind of goes without saying, but it makes us ask the question "how do I access or modify my page tables?" - we're working with virtual addresses, because paging is enabled, but we need to access physical addresses.

The obvious thing to do is to map the tables you want to modify into virtual memory when you want to modify them::

    map(KERNEL_TEMP_PAGE, my_page_table_address);
    // Do modification here on KERNEL_TEMP_PAGE
    unmap(KERNEL_TEMP_PAGE);

That's rather heavyweight though - we'd have to do this for every map and unmap operation (notice the recursiveness of that, too!).

There's a standard pattern that a lot of hobby kernels use called the *recursive page directory trick*. With this trick, you map (permanently) the page directory's address into itself.

What's the point of this?

.. image:: ../../../doc/vmm-recursive-pd-trick.svg
    :class: floated

Well, as we know, the CPU performs a two-level lookup to get the V->P mapping for a page. By short-circuiting one or two levels of lookup, we can force the MMU to only perform a one or zero-level lookup, so we can access the page directory or page tables!

Consider this: Say we want to change what page table *a* refers to, or some of its attributes. Really, we want a zero-level lookup for that - it's an index into the page directory (remember that a *page directory entry* points to a *page table*). Well, to get a zero-level lookup, we can force the MMU to traverse what are the red arrows in the picture opposite: We make it select page directory entry *m*, then entry *n* in that page table, and we end up back at the page directory again. The sum of this is we can simply::

    #define PAGE_SIZE 4096
    #define PAGE_TABLE_SIZE 4096 * 1024 // 1024 entries per page table, each size 4096.
    uint32_t *page_directory = m * PAGE_TABLE_SIZE + n * PAGE_SIZE;
    page_directory[a] |= PAGE_WRITE; // Add 'write' capability to the page table *a*.

Similarly, if we want to change what page entry *e* of page table *b* is pointing at (remember, this is a *page table entry*), we want a 1-level lookup done by the MMU. So we can force it to traverse the blue arrow in the opposite picture - the first lookup it does loops back on itself, and then it's only got one lookup left to do!::

    uint32_t *page_table_b = n * PAGE_TABLE_SIZE + b * PAGE_SIZE;
    page_table_b[e] &= ~PAGE_USER; // Remove user-level privileges.

So this is a very simple, yet somewhat counterintuitive way to manage one's V->P mappings efficiently :)

In the diagram I deliberately had two sets of arrows to follow (the blue and red ones) for ease of understanding. Really, we don't need the red arrows at all. If we define *m* to be the same as *n*, we can force the blue arrow to be navigated twice, which means we need to do less setup. So the first example becomes::

    #define PAGE_SIZE 4096
    #define PAGE_TABLE_SIZE 4096 * 1024 // 1024 entries per page table, each size 4096.
    uint32_t *page_directory = n * PAGE_TABLE_SIZE + n * PAGE_SIZE;
    page_directory[a] |= PAGE_WRITE; // Add 'write' capability to the page table *a*.
*/

/**
   We're going to set up the recursive page directory trick so that the last page directory entry (1023) is mapped back to itself.

   Later we'll be cloning an address space, and for that it is useful to be able to map a second page directory/set of page tables too, so the second-last page directory entry (1022) will be reserved for that. { */

#define RPDT_BASE  1023
#define RPDT_BASE2 1022

/**
   Now we get to our two utility macros which will hide away all the functionality of the recursive page directory trick. The algorithm is exactly as in the examples above, where ``n`` has been substituted for ``base``. { */

#define PAGE_SIZE 4096U
#define PAGE_TABLE_SIZE (PAGE_SIZE * 1024U)
#define PAGE_TABLE_ENTRY(base, v) (uint32_t*)(base*PAGE_TABLE_SIZE + \
                                              ((v)>>12) * 4)
#define PAGE_DIR_ENTRY(base, v) (uint32_t*)(base*PAGE_TABLE_SIZE + \
                                            RPDT_BASE*PAGE_SIZE + \
                                            ((v)>>22) * 4)

/**
Now we should start defining the most useful function: ``map``. ``map`` will add a virtual->physical mapping. Firstly though, it must check if the page table it wants to use has actually been created! For this, it uses the helper function ``ensure_page_table_mapped()``.

This function firstly gets a pointer to the page directory using the ``MMAP_PAGE_DIR`` constant. It checks if the n'th entry is present - if not, it allocates a new page and maps it.

TODO symbiotic relationship between vmm and pmm {*/

static void ensure_page_table_mapped(uintptr_t v) {
  if (((*PAGE_DIR_ENTRY(RPDT_BASE, v)) & X86_PRESENT) == 0) {
    dbg("ensure_page_table_mapped: alloc_page!\n");
    uint64_t p = alloc_page(PAGE_REQ_UNDER4GB);
    dbg("alloc_page finished!\n");
    if (p == ~0ULL)
      panic("alloc_page failed in map()!");

    *PAGE_DIR_ENTRY(RPDT_BASE, v) = p | X86_PRESENT | X86_WRITE | X86_USER;

    /* Ensure that the new table is set to zero first! */
    v = (v >> 22) << 22; /* Clear the lower 22 bits. */
    
    memset(PAGE_TABLE_ENTRY(RPDT_BASE, v), 0, 0x1000);
  }
}

/** The next helper function merely performs a mapping of one page. You can ignore the code referring to "cow" (copy-on-write) - we'll get back to that in a later chapter! { */

static int map_one_page(uintptr_t v, uint64_t p, unsigned flags) {
  dbg("map: getting lock...\n");
  spinlock_acquire(&current->lock);
  dbg("map: %x -> %x (flags %x)\n", v, (uint32_t)p, flags);
  /* Quick sanity check - a page with CoW must not be writable. */
  if (flags & PAGE_COW) {
    //cow_refcnt_inc(p);
    flags &= ~PAGE_WRITE;
  }

  ensure_page_table_mapped(v);
  dbg("map: Made sure page table was mapped.\n");

  if (*PAGE_TABLE_ENTRY(RPDT_BASE, v) & X86_PRESENT) {
    printk("*** mapping %x to %x with flags %x\n", v, (uint32_t)p, flags);
    panic("Tried to map a page that was already mapped!");
  }

  *PAGE_TABLE_ENTRY(RPDT_BASE, v) = (p & 0xFFFFF000) |
    to_x86_flags(flags) | X86_PRESENT;
  dbg("map: About to release spinlock\n");
  spinlock_release(&current->lock);
  dbg("map: released spinlock\n");
  return 0;
}

/** Finally we have our ``map`` function to write, which simply iterates across all pages
    it needs to map and calls the ``map_one_page`` helper. { */

int map(uintptr_t v, uint64_t p, int num_pages, unsigned flags) {
  for (int i = 0; i < num_pages; ++i) {
    if (map_one_page(v+i*0x1000, p+i*0x1000, flags) == -1)
      return -1;
    dbg("MAP DONE\n");
  }
  dbg("RET\n");
  return 0;
}

/** Unmapping a page is actually simpler, because we do not have to potentially map
    a page table also. { */

static int unmap_one_page(uintptr_t v) {
  spinlock_acquire(&current->lock);

  /** We do sanity checks to ensure what we're unmapping actually exists, else we'll
      get a page fault somewhere down the line... { */

  if ((*PAGE_DIR_ENTRY(RPDT_BASE, v) & X86_PRESENT) == 0)
    panic("Tried to unmap a page that doesn't have its table mapped!");

  uint32_t *pte = PAGE_TABLE_ENTRY(RPDT_BASE, v);
  if ((*pte & X86_PRESENT) == 0)
    panic("Tried to unmap a page that isn't mapped!");

  /** Again, ignore this stuff about copy-on-write, we'll cover it later. { */

  //uint32_t p = *pte & 0xFFFFF000;
  //if (*pte & X86_COW)
    //cow_refcnt_dec(p);

  /** We can simply set the entry to zero to unmap it. However, this isn't all we need to do.
      
      The CPU has a cache of page table entries, called the Translation Lookaside Buffer (TLB).
      If the page table entry we're unmapping is present in the TLB, the CPU won't know it's 
      unmapped unless we tell it.

      The X86 has an instruction for this: ``invlpg`` (invalidate page). It takes a virtual address
      as an argument, although GCC's inline assembly syntax means we need to pass it as a
      dereferenced pointer, which is why we re-cast ``v`` to a pointer type here and dereference
      it in the inline assembly statement. { */
  *pte = 0;

  /* Invalidate TLB entry. */
  uintptr_t *pv = (uintptr_t*)v;
  __asm__ volatile("invlpg %0" : : "m" (*pv));

  spinlock_release(&current->lock);
  return 0;
}

int unmap(uintptr_t v, int num_pages) {  
  for (int i = 0; i < num_pages; ++i) {
    if (unmap_one_page(v+i*0x1000) == -1)
      return -1;
  }
  return 0;
}

/** The next big thing we have to define is the page fault handler.

    When a memory access happens that faults - because a page was not
    present, perhaps, the CPU takes a page fault (#PG, interrupt 14).

    It puts the faulting address in the ``%cr2`` register, and puts
    information about the memory access in the error code of the fault.

    The error code mirrors the flags in the lower 4 bits of a page
    table entry - 

      * bit 0 set: The fault occurred on a page that *was* present.
      * bit 1 set: The fault occurred on a page that was writeable.
      * bit 2 set: The fault occurred on a page that was accessible from user mode.
      * bit 3 set: The fault occurred on a page where a reserved bit was set. 
      * bit 4 set: The fault occurred on an instruction fetch (as opposed to a data access). { */
static int page_fault(isr_regs_t *regs) {
  /* Get the faulting address from the %cr2 register. */
  uint32_t cr2 = read_cr2();

  /** Ignore this copy-on-write stuff for now. { */
  //if (cow_handle_page_fault(cr2, regs->err_code))
  //  return 0;

  /* Just print out a panic message and trap to the debugger if one
     is available. If not, ``debugger_trap()`` will just spin
     infinitely. */
  panic("*** Page fault @ 0x%08x (Err code %d)", cr2, regs->err_code);
  return 0;
}

/** The ``iterate_mappings()``, ``get_mapping()`` and ``is_mapped()`` functions
    are convenience functions for the rest of the kernel, and are pretty simple. I'm not going to bother explaining them :) { */

uintptr_t iterate_mappings(uintptr_t v) {
  while (v < 0xFFFFF000) {
    v += 0x1000;
    if (is_mapped(v))
      return v;
  }
  return ~0UL;
}

uint64_t get_mapping(uintptr_t v, unsigned *flags) {
  if ((*PAGE_DIR_ENTRY(RPDT_BASE, v) & X86_PRESENT) == 0)
    return ~0ULL;

  uint32_t *page_table_entry = PAGE_TABLE_ENTRY(RPDT_BASE, v);
  if ((*page_table_entry & X86_PRESENT) == 0)
    return ~0ULL;

  if (flags)
    *flags = from_x86_flags(*page_table_entry & 0xFFF);

  return *page_table_entry & 0xFFFFF000;
}

int is_mapped(uintptr_t v) {
  unsigned flags;
  return get_mapping(v, &flags) != ~0ULL;
}

/** Now we come on to the penultimate function in our virtual memory manager. This one sets up paging.

    It takes a set of "ranges" of memory as a parameter, which is what it uses to allocate physical memory. There is a somewhat symbiotic relationship between the virtual and physical memory managers. The physical memory manager needs to track information for every physical page, and that requires virtual memory space. The virtual memory manager needs the physical memory manager to allocate physical pages on demand at any time.

    So there is a possible deadlock if you don't implement your managers carefully:

      1. PMM: free_page() called.
      2. PMM: decides it needs more virtual memory space, calls map()
      3. VMM: map() called.
      4. VMM: decides that to service the map it needs to allocate a new page table.
      5. VMM: calls alloc_page().
      6. PMM: Deadlock! oh dear.

    To get around this, when initialising the virtual memory manager we use a different physical allocator, the "early allocator" (``early_alloc_page``) which is much simpler, and use that to preallocate page tables for the whole of kernel space (which in our case is 3GB-4GB).

    From that base, the above deadlock cannot happen.
 */
int init_virtual_memory() {
  /* Initialise the initial address space object. */
  static address_space_t a;
  /** We set up paging earlier during boot. The page directory is stored in the special register ``%cr3``, so we need to fetch it back. { */
  uint32_t d = read_cr3();
  a.directory = (uint32_t*) (d & 0xFFFFF000);

  spinlock_init(&a.lock);
  
  current = &a;

  /* We normally can't write directly to the page directory because it will
     be in physical memory that isn't mapped. However, the initial directory
     was identity mapped during bringup. */

  /* Recursive page directory trick - map the page directory onto itself. */
  a.directory[1023] = (uint32_t)a.directory | X86_PRESENT | X86_WRITE;

  /* Ensure that page tables are allocated for the whole of kernel space. */
  uint32_t *last_table = 0;
  for (uint64_t addr = MMAP_KERNEL_START; addr < MMAP_KERNEL_END; addr += 0x1000) {
    uint32_t *pde = PAGE_DIR_ENTRY(RPDT_BASE, (uint32_t)addr);
    if (pde != last_table) {
      if ((*pde & X86_PRESENT) == 0) {
        *pde = early_alloc_page() | X86_PRESENT | X86_WRITE;

        memset(PAGE_TABLE_ENTRY(RPDT_BASE, (uint32_t)addr), 0, 0x1000);
      }

      last_table = pde;
    }
  }

  /* Register the page fault handler. */
  install_isr(14, &page_fault);

  /* Enable write protection, which allows page faults for read-only addresses
     in kernel mode. We need this for copy-on-write. */
  write_cr0( read_cr0() | CR0_WP );

  return 0;
}

/**
   Finally we come to the last exported function, ``clone_address_space``.

   This function is how we implement ``fork()``, so it is important that it
   runs fast. We have to create a new page directory and populate it so that
   it is a copy of the current one.

   To do that, we can use the recursive page directory trick again, with
   a different base, so we can access the PDEs and PTEs of both the source
   and destination address spaces simultaneously! { */
int clone_address_space(address_space_t *dest, int make_cow) {
  spinlock_acquire(&global_vmm_lock);

  /* Allocate a page for the new page directory */
  uint32_t p = alloc_page(PAGE_REQ_NONE);
  
  spinlock_init(&dest->lock);
  dest->directory = (uint32_t*)p;

  /* Map the new directory temporarily in so we can populate it. */
  uint32_t base_addr = (uint32_t)PAGE_TABLE_ENTRY(RPDT_BASE2, 0);
  uint32_t base_dir_addr = (uint32_t)PAGE_DIR_ENTRY(RPDT_BASE2, 0);
  dbg("base_addr = %x\n", base_addr);
  *PAGE_DIR_ENTRY(RPDT_BASE, base_addr) = p | X86_WRITE | X86_PRESENT;
  *PAGE_TABLE_ENTRY(RPDT_BASE, base_dir_addr) = p | X86_WRITE | X86_PRESENT;
  /* FIXME: invlpg */

  /* Iterate over all PDE's in the source directory except the last 
     two which are reserved for the page dir trick. */
  for (uint32_t i = 0; i < MMAP_KERNEL_END; i += PAGE_TABLE_SIZE) {
    dbg("about to map %x, %x\n", i, (uint32_t)PAGE_DIR_ENTRY(RPDT_BASE2,i));
    /** By default every page directory entry in the new address space is the same as in the old address space. */
    *PAGE_DIR_ENTRY(RPDT_BASE2, i) = *PAGE_DIR_ENTRY(RPDT_BASE, i);
    dbg("here1\n");
    int is_user = ! IS_KERNEL_ADDR( PAGE_TABLE_SIZE * i );

    /** However, if the directory entry is present and is user-mode, we need
        to clone it to ensure that updates in the old address space don't affect
        the new address space and vice versa. { */
    /* Now we have to decide whether to copy/clone the current page table.
       We need to clone if it is present, and if it a user-mode page table. */
    if ((*PAGE_DIR_ENTRY(RPDT_BASE, i) & X86_PRESENT) && is_user) {
      dbg("here2\n");
      /* Create a new page table. */
      uint32_t p2 = alloc_page(PAGE_REQ_UNDER4GB);
      *PAGE_DIR_ENTRY(RPDT_BASE2, i) = p2 | X86_WRITE | X86_USER | X86_PRESENT;

      /* Copy every contained page table entry over. */
      for (unsigned j = 0; j < 1024; ++j) {
        uint32_t *d_pte = PAGE_TABLE_ENTRY(RPDT_BASE2, i + j * PAGE_SIZE);
        uint32_t *s_pte = PAGE_TABLE_ENTRY(RPDT_BASE,  i + j * PAGE_SIZE);

        /* If the page is user-mode and writable, make it copy-on-write. */
        if (make_cow && is_user && (*s_pte & X86_WRITE)) {
          *d_pte = (*s_pte & ~X86_WRITE) | X86_COW;
          //cow_refcnt_inc(*s_pte & 0xFFFFF000);
        }
      }
    }
  }
  dbg("about to finish\n");

  dbg("finished clone\n");
  spinlock_release(&global_vmm_lock);

  return 0;
}

/** That's it! 500 lines later we have a functioning virtual memory manager, which is one of the last parts of the core kernel (except threading) that is massively architecture dependent. */
