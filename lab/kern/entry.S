#include <inc/mmu.h>
#include <inc/memlayout.h>

#define RELOC(x) ((x) - KERNBASE)

.text
.globl _start 
_start = RELOC(entry)

.globl entry
entry:
	call demo
	movw $0x1234, 0x472

	movl $(RELOC(entry_pgdir)), %eax
	movl %eax, %cr3

	movl %cr0, %eax
	orl $(CR0_PE|CR0_PG|CR0_WP), %eax
	movl %eax, %cr0

	mov $relocated, %eax
	jmp *%eax

relocated:
	movl $0x0, %ebp

	movl $(bootstacktop), %esp

	call i386_init

spin:
	jmp spin

.data
	.p2align PGSHIFT
	.globl bootstack
bootstack:
	.space KSTKSIZE
	.globl bootstacktop
bootstacktop:


