//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "../SubScheduler.h"

namespace Core { class ResourceThread; }

struct nullSubScheduler : public Core::SubScheduler {
	virtual Core::ResourceThread *getThread() { return 0; }
	virtual void addThread(Core::ResourceThread *) { }
};

struct testSubScheduler : public Core::SubScheduler {
	Core::ResourceThread *thread;
	
	testSubScheduler() : thread(0) { }

	virtual Core::ResourceThread *getThread() {
		return 0;	// Это более универсальное решение.
	};
	
	virtual void addThread(Core::ResourceThread *t) {
		BOOST_ASSERT(thread == 0);
		thread = t;
	}
};
