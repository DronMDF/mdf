//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

// Этот достаточно примитивный тест на самом деле весьма много тестирует.
// Перво наперво мы тестируем запуск модулей.
// вторым пунктом мы тестируем передачу параметров в модуль
// третим пунктом мы тестируем функции регионов создание, биндинг, информация
// четвертым пунктом мы тестируем простой системы в ожидании

#include <MDF/Types.h>
#include <MDF/Kernel.h>
#include <MDF/KernelImp.h>

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
	laddr_t addr = 0;
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

static
void printMessage (volatile unsigned short *video, int x, int y, int color, const char *message)
{
	for (int pos = y * 80 + x; *message; pos++, message++) {
		video[pos] = (color << 8) | *message;
	}
}

static
void printChar (volatile unsigned short *video, int x, int y, int color, char symbol)
{
	const int pos = y * 80 + x;
	video[pos] = (color << 8) | symbol;
}

int main(int argc __unused__, char **argv __unused__)
{
	const int color = 15;
	volatile unsigned short *video = getVideoMemory();

	printMessage (video, 1, 24, color, "Hello new Kernel!");

	const int wt = 100 / 8;	// KERNEL_HZ / 8
	for (int i = 0; ; i++) {
		printChar (video, 25, 24, color, "|/-\\"[i % 4]);

		if (KernelWait (0, 0, wt) != SUCCESS) {
			while (1);
		}
	}

	return 0;
}
