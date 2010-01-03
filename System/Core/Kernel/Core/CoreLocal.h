//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Stub.h"
#include "Core.h"

namespace Core {

template<typename T>
T max(const T &a, const T &b)
{
	return (a > b) ? a : b;
}

template<typename T>
T min(const T &a, const T &b)
{
	return (a < b) ? a : b;
}

template<typename T>
T min(const T &a, const T &b, const T &c)
{
	return min(min(a, b), c);
}

template<typename T, typename E>
bool isSet(const T &value, const E &bits)
{
	T b = bits;
	return (value & b) == b;
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
