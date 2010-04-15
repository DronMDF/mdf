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
	virtual int Attach(Resource *resource, uint32_t access, laddr_t base);
	virtual int Modify(int param_id, const void *param, size_t param_size);
	virtual int Info(int info_id, void *info, size_t *info_size) const;

	void event(uint32_t event);

	virtual void addInstance(Instance *instance);
	void removeInstance(Instance *instance);
};

} // namespace Core
