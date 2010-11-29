//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <iostream>
#include <boost/test/unit_test.hpp>

#include "Types.h"

#include "../include/CoreLocal.h"
#include "../include/Core.h"

#include "../include/ResourceId.h"
#include "../include/Storage.h"
#include "../include/Resources.h"

using namespace std;
using namespace Core;

BOOST_AUTO_TEST_SUITE(resource_storage)

BOOST_AUTO_TEST_CASE(id_generator)
{
	BOOST_REQUIRE_NE(CoreRandom(), CoreRandom());
}

class TestResource : public ResourceId
{
public:
	using ResourceId::m_id;

	virtual Resource *asResource()
	{
		// На самом деле это не честный прием, но иначе Find выдаст не
		// то, что нужно.
		return reinterpret_cast<Resource *>(this);
	}
};

BOOST_AUTO_TEST_CASE(finding)
{
	id_t id = 0;

	{
		TestResource resource;
		resource.Register();
		id = resource.id();
		BOOST_REQUIRE_NE(id, 0);
		BOOST_REQUIRE_EQUAL(FindResource(id), reinterpret_cast<Resource *>(&resource));
	}

	BOOST_REQUIRE(FindResource(id) == 0);
}

BOOST_AUTO_TEST_CASE(finding2)
{
	Storage storage;
	id_t id = 0;

	{
		TestResource resource;
		BOOST_REQUIRE_EQUAL(resource.m_id, 0);

		resource.Register();
		id = resource.id();
		BOOST_REQUIRE_NE(id, 0);
		BOOST_REQUIRE_EQUAL(FindResource(id), reinterpret_cast<Resource *>(&resource));
	}

	BOOST_REQUIRE(storage.Find(id) == 0);
}

BOOST_AUTO_TEST_SUITE_END()
