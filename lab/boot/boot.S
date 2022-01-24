#include <inc/mmu.h>

#define LEDS	0x0ff1
#define VMODE   0x0ff2
#define SCRNX   0x0ff4
#define SCRNY   0x0ff6
#define VRAM    0x0ff8

.set PROT_MODE_CSEG, 0x08
.set PROT_MODE_DSEG, 0x10
.set CR0_PE_ON, 0x01

.global start
start:
	.code16
	cli
	cld
	xorw %ax,%ax
	movw %ax, %ds
	movw %ax, %ss
	movw %ax, %es
	

	mov $0x02, %ah
	int $0x16
	mov %al, (LEDS)
	mov $0x13, %al
	mov $0x00, %ah
	int $0x10
	movb $8, (VMODE)
	movw $320, (SCRNX)
	movw $240, (SCRNY)
	movl $0x000a0000, (VRAM)

seta20.1:
	inb $0x64, %al
	testb $0x02, %al	
	jnz seta20.1

	movb $0xd1, %al
	outb %al, $0x64

seta20.2:
	inb $0x60, %al
	testb $0x02, %al	
	jnz seta20.2

	movb $0xdf, %al
	outb %al, $0x60

	lgdt gdtdesc

	movl %cr0, %eax
	orl $CR0_PE_ON, %eax
	movl %eax, %cr0
	
	ljmp $PROT_MODE_CSEG, $protcseg

	.code32
protcseg:
	movw $PROT_MODE_DSEG, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	movl $start, %esp
	call bootmain

spin:
	jmp spin

.p2align 2
gdt:
	SEG_NULL
	SEG(STA_X|STA_R, 0x0, 0xffffffff)
	SEG(STA_W, 0x0, 0xffffffff)
gdtdesc:
	.word (gdtdesc - gdt - 1)
	.long gdt
