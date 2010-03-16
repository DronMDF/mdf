//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Kernel.h>
#include <Stub.h>
#include <Core.h>

#include "StubLocal.h"

#include "Multiboot.h"
#include "Page.h"
#include "Memory.h"
#include "Utils.h"
#include "Kernel.h"
#include "ELF.h"

extern void __init_begin;
extern void __bss_end;

// -----------------------------------------------------------------------------
// Multiboot MMAP foreach
#define MMAP_BEGIN(info) \
	((info)->mmap_addr)

#define MMAP_END(info) \
	((const MultibootMemory *)((char *)((info)->mmap_addr) + (info)->mmap_length))

#define MMAP_NEXT(entry) \
	((const MultibootMemory *)((char *)(entry) + (entry)->size + sizeof(u32)))

#define MMAP_FOREACH(entry, info) \
	for (const MultibootMemory *entry = MMAP_BEGIN(info); \
		entry < MMAP_END(info); entry = MMAP_NEXT(entry))

// -----------------------------------------------------------------------------
// Определение общего количества памяти.
sizex_t __init__ StubMultibootMemorySize (const MultibootInfo *info)
{
	if (info == 0) return 0;
	if (!isSet(info->flags, MULTIBOOT_FLAG_MMAP)) return 0;
	if (info->mmap_addr == 0 || info->mmap_length == 0) return 0;

	sizex_t size = 0;

	MMAP_FOREACH(mb, info) {
		if (mb->type != 1) continue;

		size += round(mb->base_addr + mb->length, PAGE_SIZE) -
			round_up(mb->base_addr, PAGE_SIZE);
	}

	return size;
}

// Определение места для временного хипа ядра.
void * __init__ StubMultibootGetFreeMemory(const MultibootInfo * const info,
	sizex_t size, void *startptr)
{
	if (info == 0) return 0;
	if (!isSet(info->flags, MULTIBOOT_FLAG_MMAP)) return 0;
	if (info->mmap_addr == 0 || info->mmap_length == 0) return 0;

	laddr_t base = v2laddr(startptr);

	while (1) {
		base = round_up(base, PAGE_SIZE);
	
		if (base + size >= KERNEL_TEMP_BASE)
			return 0;

		bool valid = false;	// Блок расположен нормально
		laddr_t nearbase = 0xffffffff;	// Ближайший блок.

		MMAP_FOREACH(mb, info) {
			if (mb->type != 1) continue;
			if (mb->base_addr + mb->length < base) continue;

			if (mb->base_addr <= base &&
			    base + size <= mb->base_addr + mb->length)
			{
				valid = true;
			}

			if (mb->base_addr > base && mb->base_addr < nearbase) {
				nearbase = mb->base_addr;
			}
		}

		if (!valid) {
			if (nearbase == 0xffffffff) return 0;
			base = nearbase;
			continue;
		}

		// Проверим по Multiboot info
		if (base < v2laddr(info) + sizeof(MultibootInfo) && base + size > v2laddr(info)) {
			base = v2laddr(info) + sizeof(MultibootInfo);
			continue;
		}
		
		if (isSet(info->flags, MULTIBOOT_FLAG_SYMS)) {
			if (base < info->elf_shoff + info->elf_shnum * info->elf_shentsize
				&& base + size > info->elf_shoff) 
			{
				base = info->elf_shoff + info->elf_shnum * info->elf_shentsize;
				continue;
			}
		}

		// Теперь проверить по модулям.
		if (!isSet(info->flags, MULTIBOOT_FLAG_MODS)) break;
		if (info->mods_addr == 0) break;

		laddr_t newbase = 0;
		for (unsigned int m = 0; m < info->mods_count; m++) {
			if (info->mods_addr[m].mod_end <= base) continue;
			if (info->mods_addr[m].mod_start >= base + size) continue;

			// Пересечение с модулем
			newbase = info->mods_addr[m].mod_end;
			break;
		}

		if (newbase != 0) {
			base = newbase;
			continue;
		}

		break;
	}

	return l2vptr(base);
}

unsigned int __init__ StubMultibootModulesCount (const MultibootInfo * const info)
{
	if (!isSet (info->flags, MULTIBOOT_FLAG_MODS))
		return 0;

	if (info->mods_addr == nullptr)
		return 0;

	CorePrint ("%u multiboot modules.\n", info->mods_count);

	return info->mods_count;
}

void __init__ StubMultibootMemoryInit (const MultibootInfo * const info)
{
	if (!isSet(info->flags, MULTIBOOT_FLAG_MMAP)) {
		STUB_ASSERT (!isSet(info->flags, MULTIBOOT_FLAG_MEM), "Missed memory info");

		CorePrint ("Missing MMAP, using lower/upper.\n");

		const sizex_t lower =
			round((sizex_t)(info->mem_lower) * 1024, PAGE_SIZE);
		StubCreatePageRegion (0, lower,
			RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE);

		const sizex_t upper =
			round ((sizex_t)(info->mem_upper) * 1024, PAGE_SIZE);
		StubCreatePageRegion (0x100000, upper,
			RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE);

		// Одну страничку для видео захватим.
		StubCreatePageRegion (VIDEO0_PAGE, 0x1000,
			RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE);

		return;
	}

	const MultibootMemory *mb0 = info->mmap_addr;
	const MultibootMemory *mbx = (MultibootMemory *)((char *)mb0 + info->mmap_length);

	bool xgamem = false;

	for (const MultibootMemory *mb = mb0; mb < mbx;
		mb = (MultibootMemory *)((char *)mb + mb->size + sizeof (u32)))
	{
		const paddr_t bp = round_up (mb->base_addr, PAGE_SIZE);
		paddr_t be = round (mb->base_addr + mb->length, PAGE_SIZE);

		if (bp <= VIDEO0_PAGE && be >= VIDEO0_PAGE + PAGE_SIZE) {
			// Эту память берем не зависимо от типа.
			xgamem = true;
		} else if (mb->type != 1) {
			continue;
		}

		// Память за пределами 4 гиг пока (пока нет PAE) отбрасываем.
		if (bp >= 0x100000000LL) {
			continue;
		}

		if (be > 0x100000000LL)
			be = 0x100000000LL;

		StubCreatePageRegion (bp, be - bp,
			RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE);
	}

	if (!xgamem) {
		// Если видеопамять не попала - выделим одну страницу.
		// Чтобы ядру было куда писать. маловероятно что это вообще не работает.
		StubCreatePageRegion (VIDEO0_PAGE, 0x1000,
			RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE);
	}
}

static
MultibootModule *modules __initdata__ = nullptr;

static
int modules_count __initdata__ = 0;

void __init__ StubMultibootReserveModules (const MultibootInfo * const info)
{
	modules_count = info->mods_count;
	modules = (MultibootModule *)StubMemoryAlloc(sizeof(MultibootModule) * modules_count);

	StubMemoryCopy (modules, info->mods_addr,
		sizeof(MultibootModule) * modules_count);

	laddr_t modbase = KERNEL_TEMP_BASE;

	for (int i = 0; i < modules_count; i++) {
		if (modules[i].string) {
			const size_t len = StubStringLength (modules[i].string);
			char *str = (char *)StubMemoryAlloc (len + 1);
			StubStringCopy (str, modules[i].string);
			modules[i].string = str;
		}

		StubKernelReservePages (modules[i].mod_start,
			modules[i].mod_end - modules[i].mod_start, modbase,
			PFLAG_READABLE);

		// Модифицировать месторасположение модуля.
		modules[i].mod_end = modbase + modules[i].mod_end - modules[i].mod_start;
		modules[i].mod_start = modbase;

		modbase += round_up (modules[i].mod_end - modules[i].mod_start, PAGE_SIZE);
	}
}

static
void __init__ StubMultibootModuleLoad (const laddr_t base, const size_t size,
	const char * const name)
{
	id_t id = INVALID_ID;

	struct KernelCreateRegionParam createp = {0, 0, 0};
	createp.size = size;
	createp.access = RESOURCE_ACCESS_READ;

	int rv = CoreCreate (0, RESOURCE_TYPE_REGION, &createp, sizeof (createp), &id);
	STUB_ASSERT (rv != SUCCESS, "Unable to create module entire region");

	struct KernelModifyRegionBindParam bindp = { 0, 0, 0, 0, 0 };
	bindp.offset = StubPageGetPAddr(base);
	bindp.size = size;

	STUB_ASSERT(bindp.shift != 0, "Invalid shift after init");

	rv = CoreModify (0, id, RESOURCE_MODIFY_REGION_PHYSICALBIND, &bindp, sizeof (bindp));
	STUB_ASSERT (rv != SUCCESS, "Unable to bind module entire region");

	if (!StubELFLoadModule (id, name, base)) {
		// TODO: Здесь надо создать глобальную инстанцию на этот регион.
		CorePrint ("RAW module: %s\n", name);

		rv = CoreModify (0, id, RESOURCE_MODIFY_NAME,
				 name, StubStringLength (name) + 1);
		STUB_ASSERT (rv != SUCCESS, "Unable to name module entire region");
	}

	// Отключить страницы от временной области.
	STUB_ASSERT (!isAligned(base, PAGE_SIZE), "Unaligned module?");
	StubKernelDropMemory (base, round_up(size, PAGE_SIZE));
}

void __init__ StubMultibootModulesLoad (void)
{
	for (int i = 0; i < modules_count; i++) {
		StubMultibootModuleLoad (modules[i].mod_start,
			modules[i].mod_end - modules[i].mod_start, modules[i].string);

		StubMemoryFree ((void *)(modules[i].string));
	}

	StubMemoryFree (modules);
	modules = nullptr;
	modules_count = 0;
}
