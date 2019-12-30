ARCH_OBJFILES 	= arch/x86/loader.o arch/x86/loader2.o arch/x86/hal.o 		\
		  arch/x86/vmm.o arch/x86/serialterm.o arch/x86/gdt.o 		\
		  arch/x86/idt.o arch/x86/isr_stubs.o arch/x86/isrs.o		\
		  arch/x86/irq_stubs.o arch/x86/irqs.o arch/x86/mem.o		\
		  arch/x86/timer.o arch/x86/vgaterm.o
ARCH_ASMFLAGS	= -f elf
ARCH_CFLAGS	= 
ARCH_LDFLAGS	=
