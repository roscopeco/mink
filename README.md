Mink is a minimal operating system kernel. Eventually it will be a research microkernel with a minimal server set.

Parts are based on James Malloy's JMTK tutorial kernel.

What can it do?
---------------

Currently, *not a lot*. It will boot (on x86), initialise the hardware, enable interrupts and go idle. Specifically, it will set up a sensible GDT and IDT, jump to protected mode, enable paging and set up the kernel's virtual memory space, set up the PIT for a kernel heartbeat (which currently just prints a dot every so often), enable interrupts and then go into a hlt loop.

As part of the build it will make a Grub2 rescue ISO that can be used to boot on either Bochs or Qemu, or burned to a disc if you like (and have a surplus of blank discs).

To build, you will need:

* *GCC compiled for X86 cross-compilation, elf output* - The makefile looks for *i586-elf-gcc* and *i586-elf-ld*. If you need to know how to build a cross-compiler, see Google.
* *GNU Make* - Obviously.
* *Grub2 (with grub2-mkrescue)* - To build the ISO.
* *xorriso* - Required by *grub2-mkrescue*.

You will also want either *bochs* or *qemu* unless you're planning to boot a real machine.

Note that the cross-compiler is *required* - It _will not_ build with a platform compiler. This is by design.

How to run it?
--------------

Let's say you have all of the above. Just do:

```
# make
```

Then one of:

```
# bochs
# ./qemu
```

Should have you sorted. Personally, I prefer Qemu. Bochs (at least the version I have) seems to have some issues with its PIT implementation (it's too fast).

What will it do, eventually?
----------------------------

The next steps are to get a basic bootstrap filesystem sorted, to enable the initial servers (including the executive) to be loaded, and provide a way to handle the rest of the boot process. Along the way will be VFS (in a server), memory swapping, proper ELF loading, and all the other bits and pieces.

The aim is that eventually Mink will be a fully-featured, non-POSIX (but maybe with a compatibility layer) microkernel and basic set of servers for research use.

Copyright?
----------

Mink is Copyright (c)2013 Ross Bamford. Certain files have portions from James Malloy's tutorial kernels - those portions remain the property of James Malloy. There are notes in the file headers where this is the case.

Mink is licensed under the MIT license. See LICENSE for details.

