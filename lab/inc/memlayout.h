#ifndef JOS_INC_MEMLAYOUT_H
#define JOS_INC_MEMLAYOUT_H

#ifndef __ASSEMBLER__
#include <inc/types.h>
#include <inc/mmu.h>
#endif /* not __ASSEMBLER__ */

#define KERNBASE 0xF0000000


#define KSTKSIZE	(8*PGSIZE)   		// size of a kernel stack

#ifndef __ASSEMBLER__

typedef uint32_t pte_t;
typedef uint32_t pde_t;

#endif /* !__ASSEMBLER__ */
#endif
