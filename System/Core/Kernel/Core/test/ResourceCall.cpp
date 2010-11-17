//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#include "Types.h"
#include "../include/Stub.h"
#include "../include/Core.h"
#include "../include/Kernel.h"
#include "../include/List.h"
#include "../include/Resources.h"

#include "../include/Resource.h"
#include "../include/Call.h"

#include "testProcess.h"
#include "testThread.h"

using namespace Core;
using namespace boost;

BOOST_AUTO_TEST_SUITE(call)

BOOST_AUTO_TEST_CASE(creation)
{
	testProcess process;
	testThread thread(&process);

	struct KernelCreateCallParam cp = { 0x1000 };
	id_t cid = INVALID_ID;
	int rv = CoreCreate(reinterpret_cast<Task *>(&thread),
			RESOURCE_TYPE_CALL, &cp, sizeof(cp), &cid);
	BOOST_REQUIRE_EQUAL(rv, SUCCESS);

	Resource *resource = Core::FindResource(cid);
	BOOST_REQUIRE(resource != 0);
	BOOST_REQUIRE(resource->asCall() != 0);
}

BOOST_AUTO_TEST_CASE(static_creator)
{
	struct KernelCreateCallParam cp = { 0x1000 };

	testProcess process;

	// Контроль параметров
	BOOST_REQUIRE(ResourceCall::Create(0, &cp, sizeof(cp)) == 0);
	BOOST_REQUIRE(ResourceCall::Create(&process, 0, sizeof(cp)) == 0);
	BOOST_REQUIRE(ResourceCall::Create(&process, &cp, sizeof(cp) + 1) == 0);

	scoped_ptr<Resource> resource(ResourceCall::Create(&process, &cp, sizeof(cp)));
	ResourceCall *call = resource->asCall();
	BOOST_REQUIRE(call != 0);

	BOOST_REQUIRE_EQUAL(call->getEntry(), 0x1000);
}

BOOST_AUTO_TEST_CASE(call)
{
	testProcess process;

	struct KernelCreateCallParam cp = { 0x1000 };
	Resource *call = ResourceCall::Create(&process, &cp, sizeof(cp));
	BOOST_REQUIRE(call != 0);

	Thread *thread = call->Call();
	BOOST_REQUIRE(thread != 0);
	BOOST_REQUIRE_EQUAL(thread->getEntry(), 0x1000);
}

BOOST_AUTO_TEST_SUITE_END()
