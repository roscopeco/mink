/* elf.c - Routines for dealing with Executable and Linking Format files.
 *
 * Portions copyright (c)2012 James Molloy.
 *
 * Copyright (c)2013 Ross Bamford. See LICENSE for details.
 */

#include "utils.h"
#include "elf.h"

const char *elf_lookup_symbol(uint32_t addr, elf_t *elf) {
  unsigned int i;

  for (i = 0; i < (elf->symtabsz/sizeof (elf_symbol_t)); i++) {
    if (ELF32_ST_TYPE(elf->symtab[i].info) != 0x2)
      continue;

    if ( (addr >= elf->symtab[i].value) &&
         (addr < (elf->symtab[i].value + elf->symtab[i].size)) ) {
      const char *name = (const char *) ((uint32_t)elf->strtab + elf->symtab[i].name);
      return name;
    }
  }
  return NULL;
}

