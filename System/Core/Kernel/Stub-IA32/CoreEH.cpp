//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <MDF/Stub.h>
#include <MDF/Core.h>

extern "C" {
#include "Utils.h"
}

namespace {

uint32_t ULEB128(const uint8_t **ptr)
{
	uint32_t rv = 0;

	for (int shift = 0; ; shift += 7) {
		uint32_t bval = **ptr;
		(*ptr)++;

		rv |= (bval & 0x7f) << shift;

		if ((bval & 0x80) == 0)
			break;
	}

	return rv;
}

int32_t SLEB128(const uint8_t **ptr)
{
	int32_t rv = 0;

	for (int shift = 0; ; shift += 7) {
		uint32_t bval = **ptr;
		(*ptr)++;

		if ((bval & 0x80) == 0) {
			rv |= (bval & 0x3f) << shift;

			if ((bval & 0x40) != 0) {
				rv = (0x40LL << shift) - rv;
			}

			break;
		}

		rv |= (bval & 0x7f) << shift;
	}

	return rv;
}

void ParseCIE(const void *ptr)
{
	const uint8_t *ciep = reinterpret_cast<const uint8_t *>(ptr);

	uint32_t length = *reinterpret_cast<const uint32_t *>(ciep);
	ciep += sizeof(uint32_t);

	if (length == 0xffffffff) {
		uint64_t extended_length = *reinterpret_cast<const uint64_t *>(ciep);
		ciep += sizeof(uint64_t);

		STUB_ASSERT(extended_length > 0xffffffff, "Big CIE length");
		length = extended_length;
	}

	STUB_ASSERT(*reinterpret_cast<const uint32_t *>(ciep) != 0, "Bad CIE ID");
	ciep += sizeof(uint32_t);
		
	STUB_ASSERT(*ciep != 1, "Bad CIE version");
	ciep += sizeof(uint8_t);

	const char *augmentation = reinterpret_cast<const char *>(ciep);
	ciep += StubStringLength(augmentation) + 1;

	CorePrint("EH CIE Augmentation: '%s'\n", augmentation);

	// Если augmentation содержит 'eh', то далее следует EH Data,
	// наш не содержит но в этом, по хорошему, стоит явно убедиться.
	for (const char *a = augmentation; *a != '\0'; a++) {
		STUB_ASSERT(a[0] == 'e' && a[1] == 'h', "EH Data in CIE");
	}

	const uint32_t calign = ULEB128(&ciep);
	CorePrint("EH CIE Code Alignment Factor: %u\n", calign);

	const int32_t dalign = SLEB128(&ciep);
	CorePrint("EH CIE Data Alignment Factor: %d\n", dalign);

	//const int return_reg = *ciep;
	ciep += sizeof(uint8_t);

	bool augdata = false;
	for (const char *a = augmentation; *a != '\0'; a++) {
		if (a[0] == 'z') {
			augdata = true;
		}
	}

	if (augdata) {
		const uint32_t auglength = ULEB128(&ciep);
		CorePrint("EH CIE Augmentation data length: %u\n", auglength);

		// Пока просто пропустим.
		ciep += auglength;
	}

	// Дальше идет Initial set of Call Frame Instructions.
	// Выглядит он так:
	// .byte	0xc
	// .uleb128 0x4
	// .uleb128 0x4
	// .byte	0x88
	// .uleb128 0x1
	// Как его расшифровать - непонятно.
	
	STUB_ASSERT(ciep - reinterpret_cast<const uint8_t *>(ptr) > (int)length,
		"Invalid CIE length");
}

} // local namespace


namespace __cxxabiv1 {

typedef enum {
	_URC_NO_REASON = 0,
} _Unwind_Reason_Code;

typedef enum {
} _Unwind_Action;

typedef enum {
} _Unwind_Exception_Class;

struct _Unwind_Exception {
};

struct _Unwind_Context {
};

extern "C"
void _Unwind_Resume(_Unwind_Exception *)
{
}

extern "C"
_Unwind_Reason_Code _Unwind_RaiseException(_Unwind_Exception *)
{
	return _URC_NO_REASON;
}

// Обработка исключений

struct __cxa_exception {
};

extern "C"
_Unwind_Reason_Code __gxx_personality_v0(int, _Unwind_Action,
	_Unwind_Exception_Class, _Unwind_Exception *, _Unwind_Context *)
{
	return _URC_NO_REASON;
}

// Служебная информация располагается перед исключением, но пока мы ее убрали.
extern "C"
void *__cxa_allocate_exception(size_t size)
{
	void *mem = StubMemoryAlloc(size);
	STUB_ASSERT(mem == 0, "No memory for exception");

	StubMemoryClear(mem, size);
	return reinterpret_cast<char *>(mem);
}

extern "C"
void __cxa_free_exception(void *thrown_exception)
{
	StubMemoryFree(thrown_exception);
}

namespace {

void __unexpected_handler()
{
	STUB_FATAL("Unexpected exception handler");
}

void __terminate_handler()
{
	STUB_FATAL("Terminate exception handler");
}

} // local namespace

extern "C"
void *__cxa_begin_catch(_Unwind_Exception *)
{
	return 0;
}

extern "C"
void __cxa_end_catch(void)
{
}

extern "C" void __eh_frame();

extern "C"
void __cxa_throw (void *thrown_exception, std::type_info *, void (*dtor)(void *))
{
	ParseCIE(reinterpret_cast<const void *>(__eh_frame));

	if (dtor != 0) {
		(*dtor)(thrown_exception);
	}

	STUB_FATAL("Unable to catch exception");
}

} // namespace __cxxabiv1

