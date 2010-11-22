//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "../include/SubScheduler.h"

namespace Core { class Thread; }

struct nullSubScheduler : public Core::SubScheduler {
	virtual Core::Thread *getThread() { return 0; }
	virtual void addThread(Core::Thread *) { }
};

struct testSubScheduler : public Core::SubScheduler {
	Core::Thread *thread;
	
	testSubScheduler() : thread(0) { }

	virtual Core::Thread *getThread() {
		return 0;	// Это более универсальное решение.
	}
	
	virtual void addThread(Core::Thread *t) {
		BOOST_ASSERT(thread == 0);
		thread = t;
	}
};
