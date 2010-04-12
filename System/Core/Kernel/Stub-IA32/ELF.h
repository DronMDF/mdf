//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

// Этот заголовок составлен в соответствии со спецификацией
// Executable and Linking Format (ELF) Specification Version 1.2

// Data Representation

typedef unsigned long Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned long Elf32_Off;
typedef unsigned long Elf32_Sword;
typedef unsigned long Elf32_Word;


// ELF header

// e_ident для Elf32_Ehdr
// Вообще-то эта информация из раздела ELF Identification, но ради EI_NIDENT мы
// описываем ее предварительно.
enum {
	EI_MAG0		= 0,
	EI_MAG1		= 1,
	EI_MAG2		= 2,
	EI_MAG3		= 3,
	EI_CLASS	= 4,
	EI_DATA		= 5,
	EI_VERSION	= 6,
	EI_PAD		= 7,
	EI_NIDENT	= 16,
};

typedef struct {
	unsigned char	e_ident[EI_NIDENT];
	Elf32_Half	e_type;
	Elf32_Half	e_machine;
	Elf32_Word	e_version;
	Elf32_Addr	e_entry;
	Elf32_Off	e_phoff;
	Elf32_Off	e_shoff;
	Elf32_Word	e_flags;
	Elf32_Half	e_ehsize;
	Elf32_Half	e_phentsize;
	Elf32_Half	e_phnum;
	Elf32_Half	e_shentsize;
	Elf32_Half	e_shnum;
	Elf32_Half	e_shstrndx;
} __attribute__ ((packed)) Elf32_Ehdr;

// e_type для Elf32_Ehdr
enum {
	ET_NONE		= 0,
	ET_REL		= 1,
	ET_EXEC		= 2,
	ET_DYN		= 3,
	ET_CORE		= 4,
	ET_LOPROC	= 0xff00,
	ET_HIPROC	= 0xffff,
};

// e_machine для Elf32_Ehdr
enum {
	EM_M32		= 1,
	EM_SPARC	= 2,
	EM_386		= 3,
	EM_68K		= 4,
	EM_88K		= 5,
	EM_860		= 7,
	EM_MIPS		= 8,
	EM_MIPS_RS4_BE	= 10,
};

// e_version для Elf32_Ehdr
enum {
	EV_NONE    = 0,
	EV_CURRENT = 1,
};


// ELF Identification

// e_ident[EI_CLASS] для Elf32_Ehdr
enum {
	ELFCLASSNONE	= 0,
	ELFCLASS32	= 1,
	ELFCLASS64	= 2,
};

//e_ident[EI_DATA] для Elf32_Ehdr
enum {
	ELFDATANONE	= 0,
	ELFDATA2LSB	= 1,
	ELFDATA2MSB	= 2,
};


// Sections

enum {
	SHN_UNDEF	= 0,
	SHN_LORESERVE	= 0xff00,
	SHN_LOPROC	= 0xff00,
	SHN_HIPROC	= 0xff1f,
	SHN_ABS		= 0xfff1,
	SHN_COMMON	= 0xfff2,
	SHN_HIRESERVE	= 0xffff,
};

typedef struct {
	Elf32_Word	sh_name;
	Elf32_Word	sh_type;
	Elf32_Word	sh_flags;
	Elf32_Addr	sh_addr;
	Elf32_Off	sh_offset;
	Elf32_Word	sh_size;
	Elf32_Word	sh_link;
	Elf32_Word	sh_info;
	Elf32_Word	sh_addralign;
	Elf32_Word	sh_entsize;
} __attribute__ ((packed)) Elf32_Shdr;

// sh_type для Elf32_Shdr
enum {
	SHT_NULL	= 0,
	SHT_PROGBITS	= 1,
	SHT_SYMTAB	= 2,
	SHT_STRTAB	= 3,
	SHT_RELA	= 4,
	SHT_HASH	= 5,
	SHT_DYNAMIC	= 6,
	SHT_NOTE	= 7,
	SHT_NOBITS	= 8,
	SHT_REL		= 9,
	SHT_SHLIB	= 10,
	SHT_DYNSYM	= 11,
	SHT_LOPROC	= 0x70000000,
	SHT_HIPROC	= 0x7fffffff
	// Значения не соответствуют стандарту на enum
	//SHT_LOUSER	= 0x80000000,
	//SHT_HIUSER	= 0xffffffff,
};

// Symbol Table

typedef struct {
	Elf32_Word	st_name;
	Elf32_Addr	st_value;
	Elf32_Word	st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf32_Half	st_shndx;
} __attribute__ ((packed)) Elf32_Sym;

#define ELF32_ST_BIND(i) ((i)>>4)
#define ELF32_ST_TYPE(i) ((i)&0xf)

// Program Header

typedef struct {
	Elf32_Word	p_type;
	Elf32_Off	p_offset;	// Смещение от начала файла
	Elf32_Addr	p_vaddr;	// Адрес секции в памяти
	Elf32_Addr	p_paddr;
	Elf32_Word	p_filesz;
	Elf32_Word	p_memsz;
	Elf32_Word	p_flags;
	Elf32_Word	p_align;
}  __attribute__ ((packed)) Elf32_Phdr;

// p_type для Elf32_Phdr
enum {
	PT_NULL		= 0,
	PT_LOAD		= 1,
	PT_DYNAMIC	= 2,
	PT_INTERP	= 3,
	PT_NOTE		= 4,
	PT_SHLIB	= 5,
	PT_PHDR		= 6,
	PT_LOPROC	= 0x70000000,
	PT_HIPROC	= 0x7fffffff,
};

// p_flags для Elf32_Phdr
enum {
	PF_X		= 0x1,
	PF_W		= 0x2,
	PF_R		= 0x4,
	// Значение не соответствует стандартту на enum
	//PF_MASKPROC	= 0xf0000000,
};

// -----------------------------------------------------------------------------
// Прототипы функций
void StubELFScanSections (Elf32_Shdr *sh, int num, int entsize);
bool StubELFLoadModule (const id_t id, const char * const name, const laddr_t addr);
