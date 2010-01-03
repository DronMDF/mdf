//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"

#include "ResourceId.h"
#include "Resource.h"
#include "ResourceInstance.h"
#include "Memory.h"
#include "Thread.h"
#include "Event.h"

#include "CoreLocal.h"

namespace Core {

Resource::Resource (void)
	: ResourceId(),
	  m_name(0),
	  m_namelen(0),
	  m_instances_count(0),
	  m_event(0)
{
}

Resource::~Resource (void)
{
	delete[] m_name;
	delete m_event;
}

int Resource::getInstancesCount() const
{
	return m_instances_count;
}

Resource *Resource::asResource()
{
	return this;
}

ResourceRegion *Resource::asRegion()
{
	return 0;
}

ResourceProcess *Resource::asProcess()
{
	return 0;
}

ResourceThread *Resource::asThread ()
{
	return 0;
}

ResourceCall *Resource::asCall ()
{
	return 0;
}

ResourceCustom *Resource::asCustom ()
{
	return 0;
}


void Resource::SetName (const unsigned char * const name, const size_t namelen)
{
	// TODO: Стоит убедиться в том, что другого такого ресурса не существует.
	// Всмысле убедиться в уникальности имени.

	// НО с другой стороны если доступ только через глобальные инстанции,
	// то кому поможет имя ресурса, у которого нету инстанций?
	// Сможешь узнать идентификатор, но воспользоваться им не сможешь.

	m_namelen = namelen;
	delete[] m_name;
	m_name = 0;

	if (m_namelen != 0) {
		m_name = new unsigned char [m_namelen];
		StubMemoryCopy (m_name, name, m_namelen);
	}
}

ResourceThread *Resource::Call ()
{
	return 0;
}

int Resource::Attach (Resource *, int, unsigned long)
{
	return ERROR_INVALIDID;
}

int Resource::Modify (int param_id, const void *param, size_t param_size)
{
	switch (param_id) {
		case RESOURCE_MODIFY_NAME:
			SetName (static_cast<const unsigned char *>(param), param_size);
			return SUCCESS;

		default:
			break;
	}

	return ModifyIndependent(param_id, param, param_size);
}

int Resource::Info (int info_id, void *info, size_t *info_size) const
{
	return InfoIndependent(info_id, info, info_size);
}

ResourceInstance *Resource::CreateInstance (int capability, unsigned long param)
{
	ResourceInstance *instance = new ResourceInstance (this, capability, param);
	++m_instances_count;
	return instance;
}

void Resource::DeleteInstance ()
{
	--m_instances_count;
	if (m_instances_count == 0)
		delete this;
}

void Resource::addObserver(ResourceThread *thread, uint32_t event)
{
	if (m_event == 0) {
		m_event = new Event;
	}

	m_event->addObserver(thread, event);
}

void Resource::setEvent(uint32_t event)
{
	if (m_event == 0) {
		m_event = new Event;
	}

	m_event->Action(event);
}

} // namespace Core
