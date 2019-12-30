ARCH_OBJFILES 	= arch/x86_64/loader.o arch/x86_64/loader64.o arch/x86_64/loader2.o	\
		  arch/x86_64/vgaterm.o arch/x86_64/hal.o arch/x86_64/vmm.o		\
		  arch/x86_64/idt.o arch/x86_64/isr_stubs.o arch/x86_64/isrs.o
ARCH_ASMFLAGS	= -f elf64
ARCH_CFLAGS	= -mno-red-zone
ARCH_LDFLAGS	=
