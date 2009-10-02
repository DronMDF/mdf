//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

namespace Core {

class InactiveScheduler : public SubScheduler
{
private:
	ThreadList m_imminent;

protected:
	// TODO: Надо сделать возможночть отвязывать объект от очереди через Link
	//	Тогда эту очередь в приват.
	ThreadList m_infinity;

	virtual bool checkThreadUrgency(const ResourceThread *thread,
		const ResourceThread *exist) const;

public:
	InactiveScheduler();
	virtual ~InactiveScheduler();

	virtual void addThread(ResourceThread *thread);
	virtual ResourceThread *getThread();
};

} // namespace Core
