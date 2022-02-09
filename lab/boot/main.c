#include <inc/x86.h>
#include <inc/elf.h>

#define SECTSIZE 512
#define ELFHDR ((struct Elf*)0x10000)

void readsect(void*, uint32_t);
void readseg(uint32_t, uint32_t, uint32_t);

void bootmain(void)
{
	struct Proghdr *ph, *eph;
    //from hard disk read 4kb to memry 0x10000	
	readseg((uint32_t)ELFHDR, SECTSIZE * 8, 0);

	if (ELFHDR->e_magic != ELF_MAGIC)
		goto bad;

	ph = (struct Proghdr *)((uint8_t *)ELFHDR + ELFHDR->e_phoff);
	
	eph = ph + ELFHDR->e_phnum;
	
	for (; ph < eph; ph++)
		readseg(ph->p_pa, ph->p_memsz, ph->p_offset);

	((void(*)(void))(ELFHDR->e_entry))();
bad:
	outw(0x8A00, 0x8A00);
	outw(0x8A00, 0x8E00);
	while(1);
}

void readseg(uint32_t pa, uint32_t count, uint32_t offset)
{
	uint32_t end_pa;

	end_pa = pa + count;
	// round down to sector boundary
	pa &= ~(SECTSIZE - 1);
	// translate from bytes to sectors and kernel starts at sector 1
	offset = (offset / SECTSIZE) + 1;

	while (pa < end_pa) {
		readsect((uint8_t*)pa, offset);
		pa += SECTSIZE;
		offset++;
	}
}

void waitdisk(void)
{
	while ((inb(0x1F7) & 0xC0) != 0x40);
}

void readsect(void *dst, uint32_t offset)
{
	waitdisk();
	// 0x1f2存放要读写的扇区数量
	outb(0x1F2, 1);
	// 0x1f3 存放要读写的扇区号码，就是偏移量
	outb(0x1F3, offset);
	// 0x1f4 存放读写柱面的低8位字节
	outb(0x1F4, offset >> 8);
	// 0x1f5 存放读写柱面的高2位字节
	outb(0x1F5, offset >> 16);
	// 0x1f6 存放要读写的磁盘号,磁头号，
	outb(0x1F6, (offset >> 24) | 0xE0);
	outb(0x1F7, 0x20);

	waitdisk();
	insl(0x1F0, dst, SECTSIZE / 4);
}
