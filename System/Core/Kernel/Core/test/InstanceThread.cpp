//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "testResource.h"
#include "testThread.h"
#include "TestHelpers.h"

namespace Core {
	class Instance;
}

using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteInstance)

BOOST_AUTO_TEST_CASE(testCreateInstance)
{
	testThread thread;
	struct inResource : public Resource, private visit_mock {
		virtual void addInstance(Instance *instance) {
			visit(); Resource::addInstance(instance);
		}
	} resource;
	thread.Wait(&resource, 666);	// Инстанция создается здесь
}

struct inThread : public testThread, private visit_mock {
	virtual void Activate() { visit(); }
};

BOOST_AUTO_TEST_CASE(testDeliveryWanted)
{
	inThread thread;
	testResource resource;
	
	thread.Wait(&resource, 666);	// Инстанция создается здесь
	resource.setEvent(666);		// Здесь она должна дернуть метод thread->Activate
}

BOOST_AUTO_TEST_CASE(testDeliveryDie)
{
	inThread thread;
	testResource resource;

	thread.Wait(&resource, 666);	// Инстанция создается здесь
	// При смерти ресурс должен уведомить нить.
}

BOOST_AUTO_TEST_SUITE_END()
