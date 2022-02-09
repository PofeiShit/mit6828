#include <inc/memlayout.h>

#define RELOC(x) ((x) - KERNBASE)

.text
.globl _start 

_start:
	call demo

