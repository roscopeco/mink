ARCH ?= x86
CPU ?= i686
BINFMT ?= elf

CC	= $(CPU)-$(BINFMT)-gcc
CFLAGS	= -Wall -O4 -fno-omit-frame-pointer -Wextra -ffreestanding -std=c11	\
		-D__MINK_KERNEL__ -D$(ARCH) -DMINK_ASSERTIONS -Iinclude		\
		$(EXTRA_CFLAGS)
LD	= $(CPU)-$(BINFMT)-ld
LDFLAGS = -Map mink.map

MKDIR = mkdir -p
RM = rm -rf
CP = cp -r

OBJFILES = 	arch/x86_64/loader.o arch/$(ARCH)/loader2.o arch/$(ARCH)/hal.o \
		arch/$(ARCH)/vmm.o arch/$(ARCH)/serialterm.o arch/$(ARCH)/gdt.o \
		arch/$(ARCH)/idt.o arch/$(ARCH)/isr_stubs.o arch/$(ARCH)/isrs.o	\
		arch/$(ARCH)/irq_stubs.o arch/$(ARCH)/irqs.o arch/$(ARCH)/mem.o	\
		arch/$(ARCH)/timer.o arch/$(ARCH)/vgaterm.o			\
		kmain.o sys.o console.o bitmap.o buddy.o pmm.o tick.o vmspace.o \
		slab.o kmalloc.o elf.o locking.o utils.o vsprintf.o

all: mink.bin test

.PHONY: test
test:
	make -C tests

.s.o:
	nasm -f elf -o $@ $<
 
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<
  
mink.bin: $(OBJFILES)
	$(LD) $(LDFLAGS) -T arch/$(ARCH)/linker.ld -o $@ $^	

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
	make -C tests clean
 
