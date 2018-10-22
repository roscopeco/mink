/* loader.c - Mink second-stage loader.
 *
 * This is heavily influenced by the loader from James Molloy's JMTK.
 * Portions copyright (c)2012 James Molloy.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 *
 * This is the second-stage loader for Mink. Control passes here from
 * the first-stage loader, defined in loader.s. Its responsibilities are:
 */
 
#include "hal.h"
#include "utils.h"
#include "sys.h"
#include "elf.h"
#include "x86/multiboot.h"
#include "x86/vgaterm.h"

/* Give the early allocator 2KB to play with. */
#define EARLYALLOC_SZ 2048

/* this is defined in x86/hal.c */
extern elf_t kernel_elf;

/* this is defined, predictibly enough, in kmain.c */
extern int kmain(int argc, const char **argv);

/* The global multiboot struct, which will have all its pointers pointing to
   memory that has been earlyalloc()d. */
multiboot_info_t mboot;

/* The early allocator */
static uintptr_t earlyalloc(unsigned len) {
  static uint8_t buf[EARLYALLOC_SZ];
  static unsigned idx = 0;

  if (idx + len >= EARLYALLOC_SZ)
    /* Return NULL on failure. It's too early in the boot process to give out a
       diagnostic.*/
    return 0;

  uint8_t *ptr = &buf[idx];
  idx += len;

  return (uintptr_t)ptr;
}

/* extract elf_t from multiboot info */
static elf_t elf_from_multiboot(multiboot_elf_section_header_table_t *mb) {
  unsigned int i;
  elf_t elf;
  elf_section_header_t *sh = (elf_section_header_t*)mb->addr;

  uint32_t shstrtab = sh[mb->shndx].addr;
  for (i = 0; i < mb->num; i++) {
    const char *name = (const char *) (shstrtab + sh[i].name);
    if (!strcmp (name, ".strtab")) {
      elf.strtab = (const char *)sh[i].addr;
      elf.strtabsz = sh[i].size;
    }
    if (!strcmp (name, ".symtab")) {
      elf.symtab = (elf_symbol_t*)sh[i].addr;
      elf.symtabsz = sh[i].size;
    }
  }
  return elf;
}

/* Entry point from assembly. */
#if defined(__cplusplus)
extern "C"
#endif
void loader(unsigned int magic, multiboot_info_t *_mboot) {
  // setup the terminal
	vgaterm_init();

	// Check multiboot info looks good. We'll use this when we're setting up the
	// memory later on. If we've got bad info, we'll simply return. The loader 
	// will disable interrupts and halt the cpu...
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
	  // incorrect bootloader magic - can't trust multiboot info - might as well die now...
	  vgaterm_setcolor(make_color(COLOR_LIGHT_RED, COLOR_BLACK));
	  vgaterm_writestring("Failure (BADMAGIC)");
  	return;
  }

  /* Copy the multiboot struct itself. */
  memcpy((uint32_t*)&mboot, (uint32_t*)_mboot, sizeof(multiboot_info_t));

  /* If the cmdline member is valid, copy it over. */
  if (mboot.flags & MULTIBOOT_INFO_CMDLINE) {
    /* We are now operating from the higher half, so adjust the pointer to take
       this into account! */
    _mboot->cmdline += 0xC0000000;
    int len = strlen((char*)_mboot->cmdline) + 1;
    mboot.cmdline = earlyalloc(len);
    if (mboot.cmdline) {
      memcpy((uint8_t*)mboot.cmdline, (uint8_t*)_mboot->cmdline, len);
    }
  }

  if (mboot.flags & MULTIBOOT_INFO_MODS) {
    _mboot->mods_addr += 0xC0000000;
    int len = mboot.mods_count * sizeof(multiboot_module_t);
    mboot.mods_addr = earlyalloc(len);
    if (mboot.mods_addr) {
      memcpy((uint8_t*)mboot.mods_addr, (uint8_t*)_mboot->mods_addr, len);
    }
  }

  if (mboot.flags & MULTIBOOT_INFO_ELF_SHDR) {
    _mboot->u.elf_sec.addr += 0xC0000000;
    int len = mboot.u.elf_sec.num * mboot.u.elf_sec.size;
    mboot.u.elf_sec.addr = earlyalloc(len);
    if (mboot.u.elf_sec.addr) {
      memcpy((uint8_t*)mboot.u.elf_sec.addr, (uint8_t*)_mboot->u.elf_sec.addr, len);
    }
    // we need to set the global elf symbol var (in kmain.c).
    kernel_elf = elf_from_multiboot(&mboot.u.elf_sec);
  } else {
    memset(&kernel_elf, 0, sizeof(elf_t));
  }

  if (mboot.flags & MULTIBOOT_INFO_MEMORY) {
    _mboot->mmap_addr += 0xC0000000;
    mboot.mmap_addr = earlyalloc(mboot.mmap_length + 4);
    if (mboot.mmap_addr) {
      memcpy((uint8_t*)mboot.mmap_addr,
             (uint8_t*)_mboot->mmap_addr - 4, mboot.mmap_length+4);
      mboot.mmap_addr += 4;
      mboot.mmap_addr = _mboot->mmap_addr;
    }
  }
  
  
  
  static const char *argv[256];
  int argc = tokenize(' ', (char*)mboot.cmdline, argv, 256);

  (void)kmain(argc, argv);
}  

