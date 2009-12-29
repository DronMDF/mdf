//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <MDF/Stub.h>
#include <MDF/Core.h>
#include <MDF/Kernel.h>

void *operator new(unsigned int size)
{
	void *ptr = StubMemoryAlloc (size);
	STUB_ASSERT (ptr == 0, "No memory");
	return ptr;
}

void *operator new[](unsigned int size)
{
	void *ptr = StubMemoryAlloc(size);
	STUB_ASSERT(ptr == 0, "No memory");
	return ptr;
}

void operator delete(void *ptr) throw()
{
	if (ptr == 0) return;	// Допустимо по стандарту.
	CorePrint("delete 0x%08x\n", ptr);
	StubMemoryFree (ptr);
	CorePrint("delete 0x%08x done\n", ptr);
}

void operator delete[](void *ptr) throw()
{
	if (ptr == 0) return;	// Допустимо по стандарту.
	StubMemoryFree (ptr);
}

namespace __cxxabiv1 {

extern "C"
void __cxa_pure_virtual (void)
{
	STUB_FATAL ("Called pure virtual function");
}

} // namespace __cxxabiv1

