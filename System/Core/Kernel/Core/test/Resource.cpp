//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <iostream>
#include <boost/test/unit_test.hpp>

#include "Types.h"

#include "../Kernel.h"
#include "../Event.h"

#include "testResource.h"
#include "testThread.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(resource)

class eventedResource : public testResource {
public:
	eventedResource(Event *event = 0) : testResource() {
		m_event = event;
	}
};

class testEventSpy : public Event {
public:
	bool add_visited;
	bool action_visited;

	testEventSpy() : Event(), add_visited(false), action_visited(false) {}

	void addObserver(ResourceThread *, uint32_t) { add_visited = true; }
	virtual void Action(uint32_t) { action_visited = true; }
};

BOOST_AUTO_TEST_CASE(add_observer_creation)
{
	class unactivatedThread : public testThread {
		virtual void Activate() {};
	} thread;
	testResource resource;

	resource.addObserver(&thread, RESOURCE_EVENT_THREAD);

	BOOST_REQUIRE(resource.m_event != 0);
}

BOOST_AUTO_TEST_CASE(add_observer_calling)
{
	testEventSpy *event = new testEventSpy;
	eventedResource resource(event);
	resource.addObserver(0, 0);
	BOOST_REQUIRE(event->add_visited);
}

BOOST_AUTO_TEST_CASE(event_creation)
{
	testResource resource;
	resource.setEvent(RESOURCE_EVENT_THREAD);
	BOOST_REQUIRE(resource.m_event != 0);
}

BOOST_AUTO_TEST_CASE(event_action)
{
	testEventSpy *event = new testEventSpy;
	eventedResource resource(event);
	resource.setEvent(0);
	BOOST_REQUIRE(event->action_visited);
}

BOOST_AUTO_TEST_SUITE_END()
