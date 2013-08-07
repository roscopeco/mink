CC	= i586-elf-gcc
CFLAGS	= -Wall -O4 -fno-omit-frame-pointer -Wextra -ffreestanding \
					-std=c99 -D__MINK_KERNEL__ -DX86 -DMINK_ASSERTIONS -Iinclude 
LD	= i586-elf-ld
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

all: mink.iso
 
.s.o:
	nasm -f elf -o $@ $<
 
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<
  
mink.bin: $(OBJFILES)
	$(LD) $(LDFLAGS) -T arch/x86/linker.ld -o $@ $^	

# Generates the Grub2-based ISO - see bochsrc.txt for info.
# Requires grub2-mkrescue and xorriso be installed!
mink.iso: mink.bin grub.cfg
	$(MKDIR) build/iso/boot/grub
	$(CP) $< build/iso/boot
	$(CP) grub.cfg build/iso/boot/grub
	grub2-mkrescue -o mink.iso build/iso

clean:
	$(RM) $(OBJFILES) *.bin *.img mink.iso build
 
