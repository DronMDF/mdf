//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "ResourceId.h"
#include "List.h"

namespace Core {

class Instance;
class InstanceProcess;

class ResourceRegion;
class ResourceProcess;
class ResourceThread;
class ResourceCall;
class ResourceCustom;

class Event;

// Базовый класс для всех ресурсов
class Resource : private ResourceId
{
private:
	// Имя ресурса - блоб
	unsigned char *m_name;
	size_t m_namelen;

protected:	// для теста
	Event *m_event;

private:
	/// Список инстанций
	List<Instance> m_instances;

	Resource (const Resource &);
	Resource & operator = (const Resource &);

	void SetName (const unsigned char * const name, const size_t namelen);

protected:
	Resource ();

	int getInstancesCount() const;

public:
	virtual ~Resource ();

	using ResourceId::id;
	using ResourceId::Register;

	virtual Resource * asResource();
	virtual ResourceRegion *asRegion ();
	virtual ResourceProcess *asProcess ();
	virtual ResourceThread *asThread ();
	virtual ResourceCall *asCall ();
	virtual ResourceCustom *asCustom ();

	// Методы API...

	// Call сильно атрофированный метод, можно ли его приравнять к методам
	// API, уж не знаю. Да всеравно..
	virtual ResourceThread *Call ();
	virtual int Attach(Resource *resource, uint32_t access, uint32_t spec);
	virtual int Modify(int param_id, const void *param, size_t param_size);
	virtual int Info(int info_id, void *info, size_t *info_size) const;

	// Этa функции уйдет в небытие после перевода на событийные инстанции.
	virtual void addObserver(ResourceThread *thread, uint32_t event);
	void setEvent(uint32_t event);

	void addInstance(Instance *instance);
	void removeInstance(Instance *instance);
};

} // namespace Core
