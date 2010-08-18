/*
 * Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
 * This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
 */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")

OUTPUT_ARCH(i386)

ENTRY(StubEntryLo)

SECTIONS
{
	. = 0x100000;
	. += SIZEOF_HEADERS;
	. = ALIGN(16);

	.text : {
		__init_begin = .;
		*(.init.multiboot)

		*(.init.stack)
		*(.init.data)

		. = ALIGN(0x1000);
		__init_ro = .;
		*(.init.text)
		__init_end = .;

		. = ALIGN(0x1000);
		__text_begin = .;
		*(.text*)
		*(.gnu.linkonce*)
		__text_end = .;

		. = ALIGN(16);
		__rodata_begin = .;
		__eh_frame = .;
		*(.eh_frame*)
		*(.gcc_except_table*)
		*(.rodata*)
		__rodata_end = .;
	}

	. = ALIGN(0x1000);
	.data : {
		__data_begin = .;
		*(.data)
		__data_end = .;
	}

	. = ALIGN(16);
	.bss : {
		__bss_begin = .;
		*(.bss)
		__bss_end = .;
	}

	/DISCARD/ : {
		*.*;
	}
}
