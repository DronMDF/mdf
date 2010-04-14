//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"
#include "ResourceThread.h"
#include "InstanceThread.h"

using namespace Core;

InstanceThread::InstanceThread(Resource *resource, uint32_t event, ResourceThread *thread)
	: Instance(resource), m_event(event), m_thread(thread)
{
}

void InstanceThread::event(uint32_t event)
{
	// TODO: Если нить и без нас активна - прост сохраняем события.
	if (event == m_event || event == RESOURCE_EVENT_DESTROY) {
		// Подразумеваем отключение от ресурса.
		Instance::event(RESOURCE_EVENT_DESTROY);
		m_thread->Activate();
	}
}
