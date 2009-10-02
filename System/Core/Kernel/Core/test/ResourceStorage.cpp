//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <iostream>
#include <boost/test/unit_test.hpp>

#include "Types.h"

#include "../CoreLocal.h"
#include "../Core.h"

#include "../ResourceId.h"
#include "../ResourceStorage.h"
#include "../Resources.h"

using namespace std;
using namespace Core;

BOOST_AUTO_TEST_SUITE(resource_storage)

BOOST_AUTO_TEST_CASE(id_generator)
{
	BOOST_CHECK(CoreRandom() != CoreRandom());
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
		id = resource.getId();
		BOOST_CHECK(id != 0);
		BOOST_CHECK(FindResource(id) == static_cast<void *>(&resource));
	}

	BOOST_CHECK(FindResource(id) == 0);
}

BOOST_AUTO_TEST_CASE(finding2)
{
	ResourceStorage storage;
	id_t id = 0;

	{
		TestResource resource;
		BOOST_CHECK(resource.m_id == 0);

		resource.Register();
		id = resource.getId();
		BOOST_CHECK(id != 0);
		BOOST_CHECK(storage.Find(id) == static_cast<void *>(&resource));
	}

	BOOST_CHECK(storage.Find(id) == 0);
}

BOOST_AUTO_TEST_SUITE_END();
