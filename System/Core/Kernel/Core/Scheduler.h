//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

namespace Core {

class ResourceThread;
class SubScheduler;

// Этот класс конструируется где угодно а очереди у него статические.
class Scheduler {
protected:
	static SubScheduler *m_actives;
	static SubScheduler *m_inactives;
	static SubScheduler *m_killed;

public:
	Scheduler();
	virtual ~Scheduler();

	void addActiveThread(ResourceThread *thread);
	void addInactiveThread(ResourceThread *thread);
	void addKillThread(ResourceThread *thread);

	ResourceThread *getThread();
};

} // namespace Core
