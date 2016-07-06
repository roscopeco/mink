ARCH ?= X86
CPU ?= i686
BINFMT ?= elf

CC	= $(CPU)-$(BINFMT)-gcc
CFLAGS	= -Wall -O4 -fno-omit-frame-pointer -Wextra -ffreestanding \
					-std=c11 -D__MINK_KERNEL__ -D$(ARCH) -DMINK_ASSERTIONS -Iinclude
LD	= $(CPU)-$(BINFMT)-ld
LDFLAGS = -Map mink.map

MKDIR = mkdir -p
RM = rm -rf
CP = cp -r

OBJFILES = 	arch/x86/loader.o arch/x86/loader2.o kmain.o 											\
						sys.o console.o arch/x86/hal.o							 										 	\
						bitmap.o buddy.o pmm.o arch/x86/vmm.o															\
						arch/x86/serialterm.o																							\
						arch/x86/gdt.o arch/x86/idt.o								 											\
						arch/x86/isr_stubs.o arch/x86/isrs.o															\
						arch/x86/irq_stubs.o arch/x86/irqs.o															\
						arch/x86/timer.o																									\
						arch/x86/mem.o																										\
						tick.o																														\
						vmspace.o slab.o kmalloc.o																				\
						arch/x86/vgaterm.o 											 													\
						elf.o locking.o utils.o vsprintf.o

all: mink.img
 
.s.o:
	nasm -f elf -o $@ $<
 
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<
  
mink.bin: $(OBJFILES)
	$(LD) $(LDFLAGS) -T arch/x86/linker.ld -o $@ $^	

# Generates the image staging area under build/img. This is
# the directory layout that is used to make the ISO or hard-disk
# images for emulators.
.PHONY: image-staging
image-staging: mink.bin grub2/grub.cfg
	$(MKDIR) build/img/boot/grub2
	$(CP) --parents $^ build/img/boot

# Generates the Grub2-based ISO - see bochsrc.txt for info.
# Requires grub2-mkrescue and xorriso be installed!
#
# This will probably go away after a while...
mink.iso: image-staging
	grub2-mkrescue -o mink.iso build/img

# Generates a hard-disk image. See mkimage.sh.
mink.img: image-staging
	sudo sh mkimage.sh

.PHONY: clean
clean:
	$(RM) $(OBJFILES) *.bin *.img mink.iso build
 
