//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

// Этот файл составлен в соответствии с
// The Multiboot Specification version 0.6.95
// http://www.gnu.org/software/grub/manual/multiboot/

// 2 The definitions of terms used through the specification

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// 3.1.1 The layout of Multiboot header

typedef struct _MultibootHeader {
	u32	magic;
  	u32	flags;
  	u32	checksum;
} __attribute__((packed)) MultibootHeader;

// 3.1.2 The magic fields of Multiboot header

#define MULTIBOOT_HEADER_MAGIC 0x1BADB002
#define MULTIBOOT_HEADER_FLAGS 0x00000003

// 3.2 Machine state

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

// 3.3 Boot information format

enum {
	MULTIBOOT_FLAG_MEM	= 1 << 0,
	MULTIBOOT_FLAG_BOOTDEV	= 1 << 1,
	MULTIBOOT_FLAG_CMD	= 1 << 2,
	MULTIBOOT_FLAG_MODS	= 1 << 3,
	MULTIBOOT_FLAG_SYMS	= 1 << 5,
	MULTIBOOT_FLAG_MMAP	= 1 << 6,
	MULTIBOOT_FLAG_DRIVES	= 1 << 7,
	MULTIBOOT_FLAG_CONFIG	= 1 << 8,
	MULTIBOOT_FLAG_LOADER	= 1 << 9,
	MULTIBOOT_FLAG_APM	= 1 << 10,
	MULTIBOOT_FLAG_VBE	= 1 << 11
};

typedef struct _MultibootModule {
	u32	mod_start;
	u32	mod_end;
	const char *string;
	u32	reserved;
} __attribute__((packed)) MultibootModule;

typedef struct _MultibootMemory {
	u32	size;
	u64	base_addr;
	u64	length;
	u32	type;
} __attribute__((packed)) MultibootMemory;

typedef struct _MultibootInfo {
	u32	flags;		// required

	// valid if flags[0] is set
	u32	mem_lower;
	u32	mem_upper;

	// valid if flags[1] is set
	u32	boot_device;

	// valid if flags[2] is set
	const char *cmdline;

	// valid if flags[3] is set
	u32	mods_count;
	const MultibootModule *mods_addr;

	// flags[4] is not support!
	// valid if flags[5] is set (ELF section table)
	u32	elf_shnum;
	u32	elf_shentsize;
	u32	elf_shoff;
	u32	elf_shstrndx;

	// valid if flags[6] is set
	u32	mmap_length;
	const MultibootMemory *mmap_addr;

	// valid if flags[7] is set
	u32	drives_length;
	u32	drives_addr;

	// valid if flags[8] is set
	u32	config_table;

	// valid if flags[9] is set
	u32	boot_loader_name;

	// valid if flags[10] is set
	u32	apm_table;

	// valid if flags[11] is set
	u32	vbe_control_info;
	u32	vbe_mode_info;
	u32	vbe_mode;
	u32	vbe_interface_seg;
	u32	vbe_interface_off;
	u32	vbe_interface_len;
} __attribute__((packed)) MultibootInfo;

sizex_t StubMultibootMemorySize(const MultibootInfo *info);
void *StubMultibootGetFreeMemory(const MultibootInfo * const info,
	sizex_t size, void *startptr);

unsigned int StubMultibootModulesCount (const MultibootInfo * const info);
void StubMultibootReserveModules (const MultibootInfo * const info);
void StubMultibootMemoryInit (const MultibootInfo * const info);
void StubMultibootModulesLoad (void);

