#include <inc/memlayout.h>

#define CYLS    0x0ff0
#define LDES    0x0ff1
#define VMODE   0x0ff2

#define RELOC(x) ((x) - KERNBASE)

.text
.globl _start 

_start:
	call demo

