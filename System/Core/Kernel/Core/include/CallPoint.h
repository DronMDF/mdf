//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Resource.h"

namespace Core {

class CallPoint : public Resource
{
private:
	Process *m_process;
	laddr_t m_entry;

	CallPoint(Process *process, laddr_t entry);

	CallPoint();
	CallPoint(const CallPoint &);
	CallPoint & operator =(const CallPoint &);

public:
	static Resource *Create(Process *process, const void *param, size_t size);

	virtual CallPoint *asCall();

	laddr_t getEntry() const;

	virtual Thread *Call();
};

} // namespace Core;
