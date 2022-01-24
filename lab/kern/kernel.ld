/* Simple linker script for the JOS kernel.
   See the GNU ld 'info' manual ("info ld") to learn the syntax. */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
	/* Link the kernel at this address: "." means the current address */
	. = 0x00100000;

	.text : AT(0x100000) {
		*(.text)
	}

	.rodata : {
		*(.rodata)
	}


	/* The data segment */
	.data : {
		*(.data)
	}

	.bss : {
		*(.bss)
	}


}
