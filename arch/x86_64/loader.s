; First-stage Loader for Mink on x86_64.
;
; Part of the Mink project. Copyright (c)2018 Ross Bamford.
; See LICENSE for details.
;
; This loader is split into two parts - the init section (loaded low) and the
; increasingly-inaptly-named higherhalf section (loaded high, at 2GB below the top
; of the address space).
;
; Init has the following responsibilities:
;
;   Set up a 4-byte stack for early init (CPUID check)
;   Ensure long mode is available (halt if not)
;   Set up a page directory to identity-map the first 4MB at 0xC0000000 (the 3GB mark).
;   Put the CPU into paging mode.
;   Far jump to higherhalf.
;
; Higherhalf then does:
;
;   Set up a stack.
;   Push the (still loaded low) MultiBoot struct to pass to loader2.
;   Calls loader (second-stage loader, in loader2.c).
;   Disables interrupts and halts (in case loader2 returns).
;   
bits 32

MBOOT_MEM_INFO      equ 1<<1
MBOOT_HEADER_MAGIC  equ 0x1BADB002
MBOOT_FLAGS         equ MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC+MBOOT_FLAGS)

section .init
mboot:  dd      MBOOT_HEADER_MAGIC
        dd      MBOOT_FLAGS
        dd      MBOOT_CHECKSUM
        
;; Entry point from bootloader.
;; At this point EBX is a pointer to the multiboot struct.
;; EAX holds the multiboot magic num, which we'll check later
;; (in loader2) to make sure we were booted by a compliant loader.
;; In the meantime, we'll just use ESI to stash the number to free
;; up EAX for the cpuid check
global _start:function _start.end-_start
_start:
        mov     esi, eax        ; temp use esi to stash the multiboot magic...
        mov     esp, istack     ; Use the 4-byte initstack for cpuid check
        xor     ebp, ebp

        ; CPUID is available if bit 21 in FLAGS can be flipped.
        pushfd
        pop     eax
        mov     ecx, eax        ; for comparing later

        xor     eax, 1 << 21    ; flip bit 21

        push    eax             ; set FLAGS and then get it back.
        popfd                   ; If CPUID is supported, the value
        pushfd                  ; we get back will still have the
        pop     eax             ; flipped bit.

        push    ecx             ; Make sure flags is back to its
        popfd                   ; original state.

        xor     eax, ecx        ; if eax and ecx are equal, bit was
        jz      .nolongmode     ; never flipped, so no cpuid (i.e. no long mode)

        ; CPUID is available. Check it supports extended function 0x80000001
        mov     eax, 0x80000000
        cpuid
        cmp     eax, 0x80000001
        jb      .nolongmode


        ; Extended functions are supported, check long mode (bit 29 in edx after cpuid)
        mov     eax, 0x80000001
        cpuid
        test    edx, 1 << 29
        jnz     .nolongmode

        ; We have long mode

        mov     eax, pd         ; Set up a page directory
        mov     dword [eax], pt + 3 ; addrs 0x0..0x400000 = pt | WRITE | PRESENT
        mov     dword [eax+0xC00], pt + 3 ; addrs 0xC0000000..0xC0400000 = same

        ;; Loop through all 1024 pages in page table 'pt', setting them to be
        ;; identity mapped with full permissions.
        mov     edx, pt
        mov     ecx, 0          ; Loop induction variable: start at 0
        
.loop:  mov     eax, ecx        ; tmp = (%ecx << 12) | WRITE | PRESENT
        shl     eax, 12
        or      eax, 3
        mov     [edx+ecx*4], eax ; pt[ecx * sizeof(entry)] = tmp
        
        inc     ecx
        cmp     ecx, 1024       ; End at %ecx == 1024
        jnz     .loop

        mov     eax, pd+3       ; Load page directory | WRITE | PRESENT
        mov     cr3, eax        ; Store into cr3.
        mov     eax, cr0
        or      eax, 0x80000000 ; Set PG bit in cr0 to enable paging.
        mov     cr0, eax

        jmp     higherhalf

;; No long mode available, print message and halt
.nolongmode:
        mov     ecx, x64_msg_l
        lea     esi, [x64_msg_s]
        lea     edi, [0xb8000]
        rep     movsb
        hlt

.end:


section .init.data
x64_msg_s       db      'U', 0x4c, 'n', 0x4c, 's', 0x4c, 'u', 0x4c, 'p', 0x4c, 'p', 0x4c, 'o', 0x4c, \
                        'r', 0x4c, 't', 0x4c, 'e', 0x4c, 'd', 0x4c, ' ', 0x4c, 'C', 0x4c, 'P', 0x4c, 'U', 0x4c, 0xa
x64_msg_l       equ $ - x64_msg_s


section .init.bss nobits
pd:     resb    0x1000          ; Page directory
pt:     resb    0x1000          ; Page table
istack_base:
        resb    0x1000          ; 8 bytes of init stack
istack:

extern loader

;; Note that we're now defining functions in the normal .text section,
;; which means we're linked in the higher half (based at 3GB).
section .text
global higherhalf:function higherhalf.end-higherhalf
higherhalf:
        mov     eax, esi        ; put multiboot magic back into eax
        mov     esp, stack      ; Ensure we have a valid stack.
        xor     ebp, ebp        ; Zero the frame pointer for backtraces.
        push    ebx             ; Pass multiboot struct as a parameter
        push    eax             ; Pass multiboot MAGIC as a parameter
        call    loader          ; Call second-stage loader.
        cli                     ; Kernel has finished, so disable interrupts ...
        hlt                     ; ... And halt the processor.
.end:

section .bss
align 8192
global stack_base
stack_base:
        resb    0x2000
stack:

