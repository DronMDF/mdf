//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "List.h"

namespace Core {

// Все субпланировщики соответствуют интерфейсы субщедулера (который я возможно
// еще расширю).

class SubScheduler {
protected:
	typedef List<ResourceThread> ThreadList;

	// для упорядоченных очередей (не важно по какому критерию)
	// существует этот шаблонный метод.
	void addThreadOrdered(ResourceThread *thread, ThreadList *list) const;

	// Критерий упорядочивания задается этой функцией.
	// Эту функцию можно было бы сделать чисто абстрактной,
	// Но дочерние классы к этому не готовы.
	virtual bool checkThreadUrgency(const ResourceThread *thread,
		const ResourceThread *exist) const;

public:
	virtual ~SubScheduler();

	virtual void addThread(ResourceThread *thread) = 0;
	virtual ResourceThread *getThread() = 0;
};

} // namespace Core
