//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <MDF/Stub.h>
#include <MDF/Core.h>
#include <MDF/Kernel.h>

namespace __cxxabiv1 {

struct _Unwind_Exception;

extern "C"
void __cxa_call_unexpected(_Unwind_Exception *) __attribute__((noreturn));

extern "C"
void __cxa_pure_virtual (void)
{
	STUB_FATAL ("Called pure virtual function");
}

} // namespace __cxxabiv1

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
	StubMemoryFree (ptr);
}

void operator delete[](void *ptr) throw()
{
	if (ptr == 0) return;	// Допустимо по стандарту.
	StubMemoryFree (ptr);
}
