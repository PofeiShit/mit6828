#ifndef JOS_INC_MMU_H
#define JOS_INC_MMU_H

#ifdef __ASSEMBLER__
#define SEG_NULL	\
	.word 0, 0;	\
	.byte 0, 0, 0, 0
#define SEG(type, base, lim)					\
	.word (((lim) >> 12) & 0xffff), ((base) & 0xffff);	\
	.byte (((base) >> 16) & 0xff), (0x90 | (type)),		\
		(0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)
#endif

// Page directory and page table constants.
#define NPDENTRIES	1024		// page directory entries per page directory
#define NPTENTRIES	1024		// page table entries per page table

#define PGSIZE		4096		// bytes mapped by a page
#define PGSHIFT		12		// log2(PGSIZE)

#define PDXSHIFT	22		// offset of PDX in a linear address


// Page table/directory entry flags.
#define PTE_P		0x001	// Present
#define PTE_W		0x002	// Writeable

#define STA_X	0x8
#define STA_E	0x4	
#define STA_C	0x4	
#define STA_W	0x2	
#define STA_R	0x2	
#define STA_A	0x1	

// Control Register flags
#define CR0_PE		0x00000001	// Protection Enable
#define CR0_WP		0x00010000	// Write Protect
#define CR0_PG		0x80000000	// Paging

#endif


