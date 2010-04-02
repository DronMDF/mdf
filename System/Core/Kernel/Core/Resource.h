//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "ResourceId.h"

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

	// Счетчик инстанций. Ресурс не хранит обратные ссылки на инстанции,
	// Потому что освобождаются они всегда по инициативе процесса, у
	// которого есть прямые ссылки. За одним исключением. Самостоятельно
	// отцеплятся от процесса могут нити, но они имеют ссылку на процесс и
	// могут попросить его отцепить себя.
	int m_instances_count;

protected:	// для теста
	Event *m_event;

private:
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
	virtual int Attach(Resource *resource, int access, unsigned long spec);
	virtual int Modify(int param_id, const void *param, size_t param_size);
	virtual int Info(int info_id, void *info, size_t *info_size) const;

	// Работа с инстанциями.
	Instance *CreateInstance (int capability, unsigned long param = 0);
	// TODO: Эту функцию надо переименовать не знаю во что, но инстанции она не удаляет.
	void DeleteInstance();

	virtual void addObserver(ResourceThread *thread, uint32_t event);
	void setEvent(uint32_t event);
};

} // namespace Core
