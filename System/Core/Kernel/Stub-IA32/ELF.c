//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include <Kernel.h>

#include "StubLocal.h"
#include "ELF.h"
#include "Memory.h"
#include "Utils.h"
#include "SoD.h"

static
void __init__ StubELFScanSymbols (Elf32_Sym *syms, unsigned int entryes,
				  char *strs, size_t strsize)
{
	if (strs == nullptr)
		return;

	// Переписываем таблицу строк в хип
	char *strtab = (char *)StubMemoryAlloc (strsize);
	STUB_ASSERT (strtab == nullptr, "String table absent");
	StubMemoryCopy (strtab, strs, strsize);

	// Подсчитать количество символов...
	unsigned int sc = 0;
	for (unsigned int snx = 0; snx < entryes; snx++) {
		const unsigned long addr = syms[snx].st_value;
		const char *name = strtab + syms[snx].st_name;
		if (addr != 0 && *name != '\0')	{
			sc++;
		}
	}

	StubSoD_SymbolCount (sc);

	// Перенести символы в BSoD..
	for (unsigned int snx = 0; snx < entryes; snx++) {
		const unsigned long addr = syms[snx].st_value;
		const char *name = strtab + syms[snx].st_name;
		if (addr != 0 && *name != '\0')	{
			StubSoD_AddSymbol (addr, name);
		}
	}

	CorePrint ("%u symbols loaded...\n", sc);
}

void __init__ StubELFScanSections (Elf32_Shdr *sh, int num, int entsize)
{
	STUB_ASSERT (entsize != sizeof (Elf32_Shdr), "Illegal shdr size");

	// Далее сканируем секции на предмет таблицы символов
	for (int ndx = 0; ndx < num; ++ndx) {
		if (sh[ndx].sh_type == SHT_SYMTAB) {
			STUB_ASSERT (sh[ndx].sh_entsize != sizeof (Elf32_Sym),
				"Illegal sym size");

			const unsigned int cnt = sh[ndx].sh_size / sh[ndx].sh_entsize;
			const unsigned int strndx = sh[ndx].sh_link;

			if (strndx == SHN_UNDEF)
				break;

			StubELFScanSymbols ((Elf32_Sym *)(sh[ndx].sh_addr), cnt,
				(char *)(sh[strndx].sh_addr), (size_t)(sh[strndx].sh_size));

			return;
		}
	}

	CorePrint ("No symbols in kernel.\n");
}

// -----------------------------------------------------------------------------
// Загрузка модулей

static
id_t __init__ StubELFCreateProcess (const laddr_t entry)
{
	id_t pid = INVALID_ID;
	const struct KernelCreateProcessParam params = {
		.entry = entry,
	};

	int rv = CoreCreate (0, RESOURCE_TYPE_PROCESS, &params, sizeof (params), &pid);
	STUB_ASSERT (rv != SUCCESS, "Unable to create process");

	return pid;
}

static
void __init__ StubELFLoadSection (const id_t pid, const id_t rid, const Elf32_Phdr *phdr)
{
	CorePrint ("\tsection: 0x%x %b(%b) %s%s%s\n",
		phdr->p_paddr, phdr->p_memsz, phdr->p_filesz,
		(phdr->p_flags & PF_X) ? "x" : "-",
		(phdr->p_flags & PF_W) ? "w" : "-",
		(phdr->p_flags & PF_R) ? "r" : "-");

	STUB_ASSERT (phdr->p_offset % PAGE_SIZE != phdr->p_vaddr % PAGE_SIZE,
		"Unaligned section in ELF");

	uint32_t access = (uint32_t)
		((isSet(phdr->p_flags, PF_R) ? RESOURCE_ACCESS_READ : 0) |
		 (isSet(phdr->p_flags, PF_W) ? RESOURCE_ACCESS_WRITE : 0));
	// Флаг Execute тоже стоит обработать, но пока таких возможностей нету.

	const struct KernelCreateRegionParam createp = {
		.size = phdr->p_memsz,
		.access = access,
	};

	id_t sid = INVALID_ID;
	int rv = CoreCreate (0, RESOURCE_TYPE_REGION, &createp, sizeof (createp), &sid);
	STUB_ASSERT (rv != SUCCESS, "Unable to create region for section");

	const struct KernelModifyRegionBindParam bindp = {
		.id = rid,
		.offset = phdr->p_offset,
		.size = phdr->p_filesz,
		.shift = 0,
	};

	rv = CoreModify (0, sid, RESOURCE_MODIFY_REGION_REGIONBIND, &bindp, sizeof (bindp));
	STUB_ASSERT (rv != SUCCESS, "Unable to bind section region");

	// А теперь надо приаттачить его к процессу.
	rv = CoreAttach (nullptr, sid, pid, access, phdr->p_vaddr);
	STUB_ASSERT (rv != SUCCESS, "Unable to attach section region");
}

bool __init__ StubELFLoadModule (const id_t id, const char *name, laddr_t addr)
{
	const Elf32_Ehdr * const hdr = l2vptr(addr); // (const Elf32_Ehdr * const)
// TODO: addr в принципе можно не передавать, все легко определяется через id.
// 	PageInfo *page = CoreResourceGetPage (id, 0, sizeof (Elf32_Ehdr));
// 	Elf32_Ehdr *hdr = (Elf32_Ehdr *)StubPageTemporary (page);

	const unsigned char stubELFident[EI_NIDENT] = {
		[EI_MAG0]	= 0x7f,
		[EI_MAG1]	= 'E',
		[EI_MAG2]	= 'L',
		[EI_MAG3]	= 'F',
		[EI_CLASS]	= ELFCLASS32,
		[EI_DATA]	= ELFDATA2LSB,
		[EI_VERSION]	= EV_CURRENT,
	};

	if (!StubMemoryEqual (hdr->e_ident, stubELFident, EI_NIDENT)) {
		CorePrint ("Bad ELF ident\n");
		return false;
	}

	if (hdr->e_type != ET_EXEC) {
		CorePrint ("Bad ELF type\n");
		return false;
	}

	if (hdr->e_machine != EM_386) {
		CorePrint ("Bad ELF machine\n");
		return false;
	}

	if (hdr->e_version != EV_CURRENT) {
		CorePrint ("Bad ELF version\n");
		return false;
	}

	if (hdr->e_entry == 0) {
		CorePrint ("Bad ELF entry\n");
		return false;
	}

	if (hdr->e_phoff == 0 ||
	    hdr->e_phentsize != sizeof (Elf32_Phdr) ||
	    hdr->e_phnum == 0)
	{
		CorePrint ("Bad ELF program header\n");
		return false;
	}

	// TODO: Модуль нормальный. загружаем.
	CorePrint ("ELF module: %s\n", name);

	const Elf32_Phdr * const phdr = l2vptr(addr + hdr->e_phoff);

	// Проверим выравнивание секций
	for (int i = 0; i < hdr->e_phnum; ++i) {
		if (phdr[i].p_type != PT_LOAD)
			continue;

		const unsigned int align = phdr[i].p_align;
		if (phdr[i].p_vaddr % align != phdr[i].p_offset % align) {
			CorePrint ("Bad ELF section alignment\n");
			return false;
		}
	}

	const id_t pid = StubELFCreateProcess (hdr->e_entry);

	int rv = CoreModify (0, pid, RESOURCE_MODIFY_NAME, name, StubStringLength (name) + 1);
	STUB_ASSERT (rv != SUCCESS, "Unable to set process name");

	for (int i = 0; i < hdr->e_phnum; ++i) {
		if (phdr[i].p_type == PT_LOAD)
			StubELFLoadSection (pid, id, &(phdr[i]));
	}

	rv = CoreCall (0, pid, (char *)name, StubStringLength (name) + 1,
		RESOURCE_CALL_ASYNC | RESOURCE_CALL_READONLY | RESOURCE_CALL_COPY);
	STUB_ASSERT (rv != SUCCESS, "Unable to run process");

	return true;
}
