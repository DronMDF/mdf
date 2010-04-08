//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Stub.h"
#include "Core.h"

namespace Core {

template<typename T> T max(const T &a, const T &b) {
	return (a > b) ? a : b;
}

template<typename T> T min(const T &a, const T &b) {
	return (a < b) ? a : b;
}

template<typename T> T min(const T &a, const T &b, const T &c) {
	return min(min(a, b), c);
}

template<typename T> bool isSet(const T &value, const T &bits) {
	return (value & bits) == bits;
}

// Разрешение неоднозначностей
inline uint32_t min(uint64_t a, uint32_t b) {
	if (a > 0xffffffffU) return b;
	return (uint32_t(a) < b) ? uint32_t(a) : b;
}

inline bool isSet(uint32_t value, int bits) {
	STUB_ASSERT(bits < 0, "Bit operation with negative");
	return (value & uint32_t(bits)) == uint32_t(bits);
}

void InitUtils (void);
void InitResources (void);

class Lock
{
private:
	lock_t * const _lock;

	// Эти у меня и не должны быть реализованы... требование effective-c++
	Lock (const Lock &lock);
	Lock & operator = (const Lock &lock);
public:
	explicit Lock (lock_t * const lock)
		: _lock(lock)
	{
		StubLock (_lock);
	}

	~Lock ()
	{
		StubUnlock (_lock);
	}
};

int ModifyIndependent (int param_id, const void *param, size_t param_size);
int InfoIndependent (int info_id, void *info, size_t *info_size);

} // namespace Core
