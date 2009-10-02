//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <MDF/Types.h>
#include <MDF/Kernel.h>
#include <MDF/KernelImp.h>

#include <stdio.h>
#include <string.h>

char id[] = "MDFVER: System-Core/KernelInfo-" VERSION;

static
volatile void *getVideoMemory ()
{
	// Создаем
	id_t rid = INVALID_ID;

	const struct KernelCreateRegionParam cpar = {
		.offset = 0,
		.size = 4000,
		.access = RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE,
	};

	if (KernelCreate (RESOURCE_TYPE_REGION, &cpar, sizeof (cpar), &rid)
		!= SUCCESS)
	{
		while (1);
	}

	// Биндим на физическую видеопамять
	const struct KernelModifyRegionBindParam mpar = {
		.id = 0,
		.offset = 0xb8000,
		.size = 4000,
		.shift = 0,
	};

	if (KernelModify(rid, RESOURCE_MODIFY_REGION_PHYSICALBIND, &mpar,
			 sizeof(mpar)) != SUCCESS)
	{
		while (1);
	}

	// Подключаем к процессу, адрес пусть выберется автоматически
	laddr_t addr = 0;	// Что-то он ее неправильно оптимизирует.
	if (KernelModify(rid, RESOURCE_MODIFY_REGION_MAP, &addr, sizeof(laddr_t)) != SUCCESS)
	{
		while (1);
	}

	// Получаем адрес замапленного региона
	size_t addr_size = sizeof(laddr_t);
	if (KernelInfo(rid, RESOURCE_INFO_REGION_INSTANCE_ADDR, &addr, &addr_size) != SUCCESS)
	{
		while (1);
	}

	return (void *)addr;
}


int main (int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	// Повышу приоритет до максимума (разумного)
	// Всеравно он только раз в секунду дергается...
	// но приложения не должны его забивать!
	id_t thread = INVALID_ID;
	size_t idsize = sizeof (id_t);
	KernelInfo(0, RESOURCE_INFO_THREAD_CURRENT, &thread, &idsize);

	uint32_t priority = 256;
	KernelModify(thread, RESOURCE_MODIFY_THREAD_PRIORITY, &priority, sizeof(uint32_t));

	volatile uint16_t *vbuf = (volatile uint16_t *)getVideoMemory();

	struct KernelInfoMemory Info;
	size_t InfoSize = sizeof (Info);

	while (1) {
		char buf[81] = {"Kernel memory info is not supported"};

		if (KernelInfo (0, RESOURCE_INFO_MEMORY, &Info, &InfoSize) == SUCCESS) {
			snprintf (buf, 81,
				"Physical: %u/%u KiB  Kernel: %u KiB  Heap: %u/%u KiB",
//				"Physical: %7u/%7u Kib  Kernel: %7u Kib  Heap: %7u/%7u Kib", // Так правильнее, но libc пока не поддерживает.
				(Info.MemoryUsed + 512) / 1024,
				(Info.MemoryTotal + 512) / 1024,
				(Info.KernelMemoryUsed + 512) / 1024,
				(Info.KernelHeapUsed + 512) / 1024,
				(Info.KernelHeapTotal + 512) / 1024);
		}

		for (int i = 0; i < 80; i++) {
			vbuf[i] = buf[i] | 0xf00;
		}

		KernelWait(0, 0, 1000);
	}

	return 0;
}
