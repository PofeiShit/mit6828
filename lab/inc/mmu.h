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

#define STA_X	0x8
#define STA_E	0x4	
#define STA_C	0x4	
#define STA_W	0x2	
#define STA_R	0x2	
#define STA_A	0x1	
#endif


