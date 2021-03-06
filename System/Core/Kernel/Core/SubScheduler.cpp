//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/List.h"
#include "include/Memory.h"
#include "include/Resource.h"
#include "include/Thread.h"
#include "include/SubScheduler.h"

using namespace Core;

SubScheduler::~SubScheduler()
{
}

void SubScheduler::addThreadOrdered(Thread *thread, SubScheduler::ThreadList *list) const
{
	if (list->getFirst() == 0) {
		list->Insert(thread);
		return;
	}
	
	for (Thread *ethread = list->getFirst(); ;
		ethread = list->getNext(ethread))
	{
		if (checkThreadUrgency(thread, ethread)) {
			list->InsertBefore(thread, ethread);
			return;
		}

		if (list->getNext(ethread) == 0) {
			list->InsertAfter(thread, ethread);
			return;
		}
	}
}

bool SubScheduler::checkThreadUrgency(const Thread *,
	const Thread *) const
{
	return true;
}
