//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../Kernel.h"
#include "../Instance.h"
#include "../InstanceProcess.h"
#include "testResource.h"
#include "TestHelpers.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(resource)

BOOST_AUTO_TEST_CASE(testDropInstances)
{
	// Инстанции не должны удалятся ресурсом. Он просто должен уведомить
	// инстанции о том, что ресурс, который они контролировали - уже умер.
	struct testInstance : public Instance, private visit_mock {
		testInstance(Resource *resource) : Instance(resource, 0) { }
		~testInstance() { BOOST_REQUIRE(resource() == 0); }
		void event(uint32_t eid) {
			BOOST_REQUIRE_EQUAL(eid, RESOURCE_EVENT_DESTROY);
			visit();
			Instance::event(eid);
		}
	};
	
	Resource *resource = new testResource;

	testInstance instance1(resource);
	testInstance instance2(resource);
	testInstance instance3(resource);

	// keeeper при удалении должен утащить ресурс за собой.
	InstanceProcess keeper(resource, 0, 0);

	// А остальные при удалениии поглядят - отвалился ресурс или нет.
}

BOOST_AUTO_TEST_SUITE_END()
