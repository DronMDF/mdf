//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <iostream>

#include <Kernel.h>

extern "C" {
#include "../Multiboot.h"
}

using namespace std;

BOOST_AUTO_TEST_SUITE(multiboot)

BOOST_AUTO_TEST_CASE(memory_size)
{
	BOOST_CHECK(StubMultibootMemorySize(0) == 0);

	MultibootInfo info;
	info.flags = 0;
	BOOST_CHECK(StubMultibootMemorySize(&info) == 0);

	info.flags = MULTIBOOT_FLAG_MMAP;
	info.mmap_addr = 0;
	BOOST_CHECK(StubMultibootMemorySize(&info) == 0);

	// блок от 0x1 до 0x1fff не содержит полных страниц
	const MultibootMemory mmap1[] =
		{ { sizeof(MultibootMemory) - sizeof(u32), 1, 0x1ffe, 1 } };

	info.mmap_length = 0;
	info.mmap_addr = &mmap1[0];
	BOOST_CHECK(StubMultibootMemorySize(&info) == 0);

	// Блоки округляются до страниц...
	info.mmap_length = sizeof(mmap1);
	BOOST_CHECK(StubMultibootMemorySize(&info) == 0);

	// И на последок реальный mmap
	const MultibootMemory mmap2[] =
		{ { sizeof(MultibootMemory) - sizeof(u32), 0, 0x9FC00, 1 },
		  { sizeof(MultibootMemory) - sizeof(u32), 0x9FC00, 0x400, 2 },
		  { sizeof(MultibootMemory) - sizeof(u32), 0xe8000, 0x18000, 2 },
		  { sizeof(MultibootMemory) - sizeof(u32), 0x100000, 0x3EF0000, 1 },
		  { sizeof(MultibootMemory) - sizeof(u32), 0x3FF0000, 0x10000, 3 },
		  { sizeof(MultibootMemory) - sizeof(u32), 0xFFFC0000, 0x40000, 2 } };

	info.mmap_length = sizeof(mmap2);
	info.mmap_addr = &mmap2[0];
	BOOST_CHECK(StubMultibootMemorySize(&info) == 0x3EF0000 + 0x9F000);
}

BOOST_AUTO_TEST_CASE(memory_place)
{
	BOOST_CHECK(StubMultibootGetFreeMemory(0, 0, 0) == 0);

	MultibootInfo info;
	info.flags = 0;
	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0, 0) == 0);

	info.flags |= MULTIBOOT_FLAG_MMAP;
	info.mmap_addr = 0;
	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0, 0) == 0);

	const MultibootMemory mmap1[1] = {};
	info.mmap_length = 0;
	info.mmap_addr = &mmap1[0];
	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0, 0) == 0);

	const MultibootMemory mmap2[] =
		{{ sizeof(MultibootMemory) - sizeof(u32), 0x100000, 0x3F00000, 1 }};
	info.mmap_length = sizeof(mmap2);
	info.mmap_addr = &mmap2[0];
	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0, 0) == reinterpret_cast<void *>(0x100000));

	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0xa0000, reinterpret_cast<void *>(0xf0000)) == reinterpret_cast<void *>(0x100000));
	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0xa0000, reinterpret_cast<void *>(0x3ff0000)) == 0);

	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0, reinterpret_cast<void *>(0x100345)) == reinterpret_cast<void *>(0x101000));

	const MultibootMemory mmap3[] =
		{{ sizeof(MultibootMemory) - sizeof(u32), 0, 0x98000, 1 },
		 { sizeof(MultibootMemory) - sizeof(u32), 0x100000, KERNEL_TEMP_BASE + 0x100000, 1 }};
	info.mmap_length = sizeof(mmap3);
	info.mmap_addr = &mmap3[0];
	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0x90000, 0) == 0);
	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0xa0000, 0) == reinterpret_cast<void *>(0x100000));

	const MultibootModule module[] =
		{{ 0x60000, 0x70000, 0, 0 },
		 { 0x160000, 0x170000, 0, 0 }};
	info.flags |= MULTIBOOT_FLAG_MODS;
	info.mods_addr = &module[0];
	info.mods_count = 1;
	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0x90000, 0) == reinterpret_cast<void *>(0x100000));

	info.mods_count = 2;
	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0x90000, 0) == reinterpret_cast<void *>(0x170000));

	// Не должен выходить на KERNEL_TEMP_BASE
	BOOST_CHECK(StubMultibootGetFreeMemory(&info, 0xa0000, reinterpret_cast<void *>(KERNEL_TEMP_BASE - 0x50000)) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
