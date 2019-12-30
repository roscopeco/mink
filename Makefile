ARCH ?= x86_64
CPU ?= $(ARCH)
BINFMT ?= elf

CC	= $(CPU)-$(BINFMT)-gcc
CFLAGS	= -O4 -fno-omit-frame-pointer -ffreestanding -std=c11			\
		-Wall -Wextra -Wpedantic -Werror				\
		-D__MINK_KERNEL__ -D$(ARCH) -DMINK_ASSERTIONS -Iinclude		\
		$(ARCH_CFLAGS)							\
		$(EXTRA_CFLAGS)

LD	= $(CPU)-$(BINFMT)-ld
LDFLAGS = -Map mink.map $(ARCH_LDFLAGS) $(EXTRA_LDFLAGS)

MKDIR = mkdir -p
RM = rm -rf
CP = cp -r

include arch/$(ARCH)/arch.mk

OBJFILES = 	$(ARCH_OBJFILES)						\
		kmain.o sys.o console.o bitmap.o buddy.o pmm.o tick.o vmspace.o \
		slab.o kmalloc.o elf.o locking.o utils.o vsprintf.o

all: mink-$(ARCH).bin test

.PHONY: test
test:
	make -C tests

.s.o:
	nasm $(ARCH_ASMFLAGS) -o $@ $<
 
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<
  
mink-$(ARCH).bin: $(OBJFILES)
	$(LD) $(LDFLAGS) -T arch/$(ARCH)/linker.ld -o $@ $^	

# Generates the image staging area under build/img. This is
# the directory layout that is used to make the ISO or hard-disk
# images for emulators.
.PHONY: image-staging
image-staging: mink-$(ARCH).bin grub2/grub.cfg
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
 
