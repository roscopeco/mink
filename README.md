> **note**: This repository is archived and development / support is discontinued.

Mink is a minimal operating system kernel. Eventually it will be a research/toy microkernel with a minimal server set.

Parts are based on James Malloy's excellent JMTK tutorial kernel, which I can thoroughly recommend to anyone interested in this stuff and wanting to learn.

Mink is licensed under the MIT license - see LICENSE for details. Copyright (c)2013-2018 Ross Bamford.

What can it do?
---------------

Currently, *not a lot*. It will boot (on x86), initialise the hardware, enable interrupts and go idle. Specifically, it will set up a sensible GDT and IDT, jump to protected mode, enable paging and set up the kernel's virtual memory space, set up the PIT for a kernel heartbeat (which currently just prints a dot every so often) enable interrupts and then go into a hlt loop.

It's pretty much ready for ring 3 at this point - the immediate next steps are to give it some notion of processes, and implement basic multitasking. After that, it'll be on to the cool stuff outlined later on in this document (VFS, networking, self-hosting, self-awareness, attempting to enslave humanity, etc). 

~~As part of the build you'll get a hard-disk image containing Grub2 and the kernel, to allow you to boot on either Bochs or Qemu. You'll also get a Grub2 rescue ISO that will boot on either, or can be burned to a disc if you like (and have a surplus of blank discs).~~
The hard-disk image is ~~the newer method for booting emulators~~ broken, and ~~should~~ should not be used ~~over the ISO which will probably be removed at some point~~ at all at present.

To build, you will need:

* *NASM* - Because I try to avoid GAS syntax where possible...
* *GCC (and binutils et. al) compiled for X86 cross-compilation, elf output* - The makefile looks for *i586-elf-gcc* and *i586-elf-ld*. If you need to know how to build a cross-compiler toolchain, see Google.
* *GNU Make* - Obviously.
* ~~*Grub2 (with grub2-mkrescue and grub2-install)* - To build the ISO and disk image.~~
* ~~*xorriso* - Required by *grub2-mkrescue*.~~
* ~~*GNU Parted* - Used to make the hard-disk image.~~
* ~~*kpartx* - Used to make the hard-disk image.~~

You will also want either *bochs* or *qemu* unless you're planning to boot a real machine. 

**Note that qemu is now the recommended emulator, mink is no longer tested under bochs.** The provided `bochsrc.txt` still works (as of August 2016) but may go away in future. If you want to use bochs with the provided configuration, you'll have to run `make mink.iso` instead of `make mink.img` as specified below.

On some distributions, you might find that grub2-xxx is named grub-xxx - if so, you'll need to edit the makefile and mkimage.sh appropriately.

Note that the cross-compiler is *required* - Mink _will not_ build with a platform compiler. This is by design.

How to run it?
--------------

**New Note: These instructions have changed _again_ (March 2018). The disk image build is utterly broken on some more recent Linux distributions (e.g. Fedora 27) so for the time being, the recommended way to get up and running is as below. Trying to build the image is _not recommended_ - it could potentially screw up your dev machine's bootloader. It probably won't, but if it does, don't say you weren't warned...**

**Less-new Note: these instructions have changed (August 2016). The default make target now only builds the kernel binary - it does not build the hard-disk image as it did before!**

Let's say you have all of the above. Just do:

```
# make
```

And then:

```
# ./qemu-kernel
```

Should have you up and running. As well as the emulated output, you can also watch registers change and whatnot in your terminal if that's your bag.

If you want to enable debugging output, you can pass defines in `EXTRA_CFLAGS` to Make, e.g:

```
# EXTRA_CFLAGS="-DKDEBUG_ENABLED" make
```

Along with `KDEBUG_ENABLED` you can also pass `KDEBUG_PMM` to debug the physical memory manager, and `KDEBUG_VMM` to debug the x86 virtual memory manager.

**Note** that enabling memory manager debugging will generate **lots** of output. You almost certainly don't want these switched on unless you're specifically working on the memory management subsystems.

What will it do, eventually?
----------------------------

The next steps are to get a basic bootstrap filesystem sorted, to enable the initial servers (including the executive) to be loaded, and provide a way to handle the rest of the boot process. Along the way will be VFS (in a server), memory swapping, proper ELF loading, and all the other bits and pieces.

The aim is that eventually Mink will be a fully-featured, non-POSIX (but maybe with a compatibility layer) microkernel and basic set of servers for research use.

Copyright?
----------

Mink is Copyright (c)2013-2018 Ross Bamford. Certain files have portions from James Malloy's tutorial kernels - those portions remain the property of James Malloy. There are notes in the file headers where this is the case.

Mink is licensed under the MIT license. See LICENSE for details.

