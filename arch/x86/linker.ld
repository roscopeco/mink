/**#5
   You might think that we're done already, but we have yet to get our kernel to build!

   The next thing we have to do is tell the linker how to link our
   kernel. Normal application programs under linux have a lot of magic done by
   the linker - we want to stop all that happening and also do some magic of our
   own. For this, we need a linker script (\*.ld).

   Note that the format of a linker script is not the nicest in the world, so 
   I'm going to assume that you either don't care about the syntax or can sort
   of pick it up as you go along. Firstly we have to inform the linker of our
   target, which is 32-bit x86. {*/
OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")
ENTRY(_start)

/**
   Then we define how to map sections. A section is an ELF concept, and is a
   chunk of code or data. It can have a name, a physical location and a virtual
   location - that is, you can instruct the linker to create an ELF section that
   will be loaded at one address but linked as if it were at another
   address. We'll need that functionality for the higher half part of our kernel.

   Firstly we want to create the ``.init`` section, which will be linked and
   loaded at 1MB, and the special ``.init.bss`` section which contains some of
   the magic in the bringup process that has yet to be fully described. { */
SECTIONS
{
  .init 0x100000 :
  {
    PROVIDE(__start = .);
    *(.init)
  }
  .init.bss ALIGN(4096) :
  {
    *(.init.bss)
  }

  /**
     Now that the lower half stuff has been defined, we add 3GB to the link
     address (``.``) so that everything else will be linked in the higher half. { */
  . += 0xC0000000;

  /**
     All other sections (``.text``, which contains the program code,
     ``.rodata`` which contains read-only data, and ``.data`` which contains
     mutable data) should be aligned on a page boundary, and loaded at the current
     link address (which will be >3GB) minus 3GB so that they are loaded to
     addresses in the lower half. { */
  .text ALIGN(4096) : AT(ADDR(.text) - 0xC0000000)
  {
    *(.mboot)
    *(.text.unlikely .text.*_unlikely)
    *(.text.exit .text.exit.*)
    *(.text.startup .text.startup.*)
    *(.text.hot .text.hot.*)
    *(.text .stub .text.* .gnu.linkonce.t.*)
    /* .gnu.warning sections are handled specially by elf32.em.  */
    *(.gnu.warning)
  }

  .rodata ALIGN(4096) : AT(ADDR(.rodata) - 0xC0000000) {
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  }
  .data ALIGN(4096) : AT(ADDR(.data) - 0xC0000000)
  {
    PROVIDE (__startup_begin = .);
    *(.startup)
    PROVIDE (__startup_end = .);
    PROVIDE (__shutdown_begin = .);
    *(.shutdown)
    PROVIDE (__shutdown_end = .);

    *(.data .data.* .gnu.linkonce.d.*)
    SORT(CONSTRUCTORS)

    /* Hack to silence warning when using GNU Gold as the linker. */
    *(.note.gnu.gold-version)
    *(.note.gnu.build-id)
  }

  .bss ALIGN(4096) : AT(ADDR(.bss) - 0xC0000000)
  {
   *(.dynbss)
   *(.bss .bss.* .gnu.linkonce.b.*)
   *(COMMON)
    PROVIDE(__end = . - 0xC0000000);
  }

  /* Get rid of all other sections. */
  /DISCARD/ : { *(.*) }

}

