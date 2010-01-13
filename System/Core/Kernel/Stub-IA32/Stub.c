//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include <Kernel.h>

#include "StubLocal.h"
#include "Arch.h"
#include "Time.h"
#include "Multiboot.h"
#include "Utils.h"
#include "Page.h"
#include "ELF.h"
#include "Memory.h"
#include "Kernel.h"
#include "CoreABI.h"

extern void __init_begin;
extern void __init_ro;
extern void __init_end;
extern void __text_begin;
extern void __text_end;
extern void __rodata_begin;
extern void __rodata_end;
extern void __data_begin;
extern void __data_end;
extern void __bss_begin;
extern void __bss_end;

// Мультибут заголовок
MultibootHeader mbh __attribute__((section (".init.multiboot"))) = {
	.magic = MULTIBOOT_HEADER_MAGIC,
	.flags = MULTIBOOT_HEADER_FLAGS,
	.checksum = -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS),
};

// -----------------------------------------------------------------------------
// Разбор командной строки ядра
struct commands {
	const char *cmd;
	bool (*func)(const char *);
};

static
struct commands sCommands[] __initdata__ = {
	{ .cmd = "console=", .func = StubSetConsole },
	{ .cmd = nullptr },
};

static
bool __init__ StubFindCommand (const char *cl)
{
	for (int i = 0; sCommands[i].cmd != nullptr; i++) {
		const size_t len = StubStringLength (sCommands[i].cmd);
		if (StubStringEqual (cl, sCommands[i].cmd, len)) {
			sCommands[i].func (cl + len);
			return true;
		}
	}

	return false;
}

static
void __init__ StubParseCommandLine (const char *cl)
{
	while (*cl != '\0') {
		if (*cl != ' ') {
			if (StubFindCommand(cl)) {
				while (*cl != '\0' && *cl != ' ') {
					cl++;
				}
				continue;
			} else {
				// TODO: Писать содержимое команды небезопасно.
				CorePrint ("Unknown command '%s'.\n", cl);
			}
		}

		cl++;
	}

	StubSetConsole(nullptr);	// Актифируем дефолтную консоль
}

// -----------------------------------------------------------------------------
// Инициализация временного хипа.
static
void __init__ StubMemoryInit1 (const MultibootInfo * const info,
	const void * * const heap_ptr, sizex_t * const heap_size)
{
	sizex_t tmpsize = StubMultibootMemorySize (info);
	STUB_ASSERT(tmpsize == 0, "No usable memory");

	CorePrint ("Usable memory size: %lb\n", tmpsize);

	tmpsize = tmpsize / PAGE_SIZE * sizeof (PageInfo)
			+ StubMultibootModulesCount (info) * 1024
			+ 64 * 1024	// GDT
			+ 64 * 1024;	// IDT и другое

	// Округляем до 4 кил.
	tmpsize += PAGE_SIZE - 1;
	tmpsize &= PADDR_MASK;

	*heap_ptr = StubMultibootGetFreeMemory(info, tmpsize, &__bss_end);
	STUB_ASSERT(*heap_ptr == 0, "No memory for temporary heap");
	
	*heap_size = tmpsize;

	StubMemoryInitBoot ((void * const)*heap_ptr, *heap_size);
	CorePrint ("Temporary heap - %lb, starting from the address 0x%08x.\n",
		*heap_size, v2laddr(*heap_ptr));
}

// static
// void __init__ StubCPUID (void)
// {
// 	const unsigned long futures = StubGetCPUFutures();
// 	if (futures & (1 << 0)) {
// 		StubPrint ("CPU support FPU.\n");
// 	}
//
// 	if (futures & (1 << 6)) {
// 		StubPrint ("CPU support PAE.\n");
// 	}
//
// 	if (futures & (1 << 9)) {
// 		StubPrint ("CPU support APIC.\n");
// 	}
//
// 	if (futures & (1 << 13)) {
// 		StubPrint ("CPU support global pages.\n");
// 	}
//
// 	StubPrint ("But all this futures is not supported in this time.\n");
// }

static
void __init__ StubInit (const MultibootInfo * const info)
{
	// Инициализируем bss.
	StubMemoryClear (&__bss_begin, &__bss_end - &__bss_begin);

	// Инитим подсистемы
	StubInitPage();
	StubInitKernel();

	if ((info->flags & MULTIBOOT_FLAG_CMD) != 0) {
		StubParseCommandLine (info->cmdline);
	}

	// Проинициализируем временный хип
	const void *heap_ptr;
	sizex_t heap_size;
	StubMemoryInit1 (info, &heap_ptr, &heap_size);

	// Подгрузим символы, чтобы красиво работал BSoD
	if ((info->flags & MULTIBOOT_FLAG_SYMS) != 0) {
		StubELFScanSections ((Elf32_Shdr *)(info->elf_shoff),
			info->elf_shnum, info->elf_shentsize);
	}

	// Хотя сперва стоило бы проинициализировать страницы,
	// а уже потом приниматься за дескрипторные таблицы,
	// Но у нас весь фокус в том, что адреса дескрипторных таблиц,
	// в связи с переходом в страничный режим не изменятся,
	// Так что нету большой разницы когда их проинитить
	StubInitGDT();
	StubInitIDT();

	// Продетектим CPU чтобы смотреть на наличие фич
	// StubCPUID();

	// Организуем списки страниц
	StubMultibootMemoryInit (info);

	//STUB_FATAL ("Look for memory map...");

	// резервируем код ядра
	// В ините могут оказаться модифицируемые данные.
	paddr_t kptr = v2paddr(&__init_begin) & PADDR_MASK;
	StubKernelReservePages (kptr, v2paddr(&__init_ro) - kptr,
		p2laddr(kptr), PFLAG_WRITABLE);

	kptr = v2paddr(&__init_ro) & PADDR_MASK;
	StubKernelReservePages (kptr, v2paddr(&__rodata_end) - kptr,
		p2laddr(kptr), PFLAG_READABLE);

	// резервируем данные ядра
	kptr = v2paddr(&__data_begin) & PADDR_MASK;
	StubKernelReservePages (kptr, v2paddr(&__bss_end) - kptr,
		p2laddr(kptr), PFLAG_WRITABLE);

	// резервируем пространство временного хипа.
	StubKernelReservePages (v2paddr(heap_ptr), heap_size,
		v2laddr(heap_ptr), PFLAG_WRITABLE);

	// Резервируем память модулей.
	StubMultibootReserveModules (info);

	// резервируем пространство видеопамяти xga.
	StubKernelReservePages (VIDEO0_PAGE, PAGE_SIZE,
		VIDEO0_PAGE, PFLAG_WRITABLE);

	// Страничный режим
	StubPageInitMode ();

	//CorePrint ("Kernel used %b\n", StubKernelPagesCnt() * PAGE_SIZE);
	//CorePrint ("Heap reserve %b\n", StubMemoryReserve());

	StubMemoryInitWork (&__bss_end, KERNEL_TEMP_BASE - v2laddr(&__bss_end));
	//CorePrint ("Heap reserve after enlarge %b\n", StubMemoryReserve());

	StubRTTIInit();

	CoreInit ();

	StubInterruptControllerInit ();
	StubTimeInit ();

	StubMultibootModulesLoad ();
}

void StubEntry (const MultibootInfo * const info, const unsigned long magic)
{
	CorePrint ("Kernel (IA32) Stub-" VERSION " and %s\n\n", CoreVersion());

	STUB_ASSERT (magic != MULTIBOOT_BOOTLOADER_MAGIC, "Not GRUB magic");

	StubInit (info);

	// TODO: всетаки CPU нужно сделать отдельной от тасков сущностью.
	StubTaskBootstrapCreate();

	StubBootstrapCaller (v2laddr(StubBootstrapEntry), StubGetSelectorCPU(0));

	STUB_FATAL ("Stub has leave...");
}

void StubIdle();

// Эта задача запустилась с пустым стеком, который расположен в конце памяти,
// как и у всех остальных задач.
void __noreturn__ StubBootstrapEntry (void)
{
	const laddr_t ibase = v2laddr(&__init_begin) & PADDR_MASK;
	const size_t isize = v2laddr(&__text_begin) - ibase;

	StubKernelDropMemory (ibase, isize);

	// Это может проявить ошибки в расположении кода.
	CorePrint ("Free init kernel memory %b\n", isize);

	// Передать управление кому нибудь...
	CoreWait (0, 0, 0, 0);

	// Здесь цикл с охлаждением процессора
	while (1) {
		// Обработчик прерывания таймера сам отберет процессор у бутстрапной нити.
		StubIdle();
	}

	STUB_FATAL ("Bootstrap has live...");
}
