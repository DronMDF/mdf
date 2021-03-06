//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

extern "C" {

#include "../Descriptor.h"
	
volatile descriptor_t *GDT = NULL;

void __init_begin() {}
void __bss_end() {}

void StubCreatePageRegion(paddr_t, sizex_t, int)
{
}

void StubKernelReservePages(paddr_t, sizex_t, laddr_t, int)
{
}

paddr_t StubPageGetPAddr(laddr_t laddr)
{
	return laddr;
}

bool StubELFLoadModule(id_t, const char *, laddr_t)
{
	return true;
}

void StubKernelDropMemory(laddr_t, size_t)
{
}

void *StubMemoryAlloc (size_t size)
{
	return malloc(size);
}

void StubMemoryFree (void *ptr)
{
	free(ptr);
}

void StubMemoryCopy (void *dst, const void *src, size_t count)
{
	memcpy(dst, src, count);
}

void StubMemoryClear(void *dst, size_t count)
{
	memset(dst, 0, count);
}

//bool CAS(uint32_t *ptr, uint32_t old_value, uint32_t new_value)
//{
//	return __sync_bool_compare_and_swap(ptr, old_value, new_value);
//}

}
