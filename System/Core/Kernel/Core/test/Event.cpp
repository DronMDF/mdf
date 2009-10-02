//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <iostream>
#include <boost/test/unit_test.hpp>

#include "Types.h"

#include "../Kernel.h"

#include "../List.h"
#include "../Memory.h"
#include "../Resource.h"
#include "../Thread.h"
#include "../Event.h"

#include "testThread.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(event)

struct event_fixture {
	class actThread : public testThread {
	public:	bool visited;
		actThread() : testThread(), visited(false) {}
		virtual void Activate() { visited = true; };
	} thread;
	Event event;

	event_fixture() : thread(), event() {
		event.addObserver(&thread, RESOURCE_EVENT_PROCESS);
	}

	virtual ~event_fixture() {
		if (thread.EventLink.isLinked())
			thread.EventLink.Unlink(&thread);
	}
};

BOOST_FIXTURE_TEST_CASE(add_observer, event_fixture)
{
	BOOST_REQUIRE(thread.EventLink.isLinked());
	BOOST_REQUIRE_EQUAL(thread.getEvent(), RESOURCE_EVENT_PROCESS);
}

BOOST_FIXTURE_TEST_CASE(valid_action, event_fixture)
{
	// На другие события не реагирует.
	event.Action(RESOURCE_EVENT_THREAD);
	BOOST_REQUIRE(thread.EventLink.isLinked());
	BOOST_REQUIRE(!thread.visited);

	// Только на свое!
	event.Action(RESOURCE_EVENT_PROCESS);
	BOOST_REQUIRE(!thread.EventLink.isLinked());
	BOOST_REQUIRE(thread.visited);
}

BOOST_FIXTURE_TEST_CASE(destroy_action, event_fixture)
{
	// И еще реагирует на дестрой
	event.Action(RESOURCE_EVENT_DESTROY);
	BOOST_REQUIRE(!thread.EventLink.isLinked());
	BOOST_REQUIRE_EQUAL(thread.getEvent(), RESOURCE_EVENT_DESTROY);
}

BOOST_AUTO_TEST_SUITE_END()
