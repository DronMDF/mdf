//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <boost/test/unit_test.hpp>
#include <boost/static_assert.hpp>

#include "Types.h"

#include "../Link.h"
#include "../List.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(list_link)

struct testItem {
	Link<testItem> link;
	testItem() : link() {}
};

BOOST_STATIC_ASSERT(sizeof(Link<testItem>) == sizeof(void *) * 3);
BOOST_STATIC_ASSERT(sizeof(List<testItem>) == sizeof(void *) * 2);

struct insert_fixture {
	List<testItem> list;
	testItem item1;
	testItem item2;
	testItem item3;

	insert_fixture() : list(&testItem::link), item1(), item2(), item3() {}

	virtual ~insert_fixture() {
		item1.link.Unlink(&item1);
		item2.link.Unlink(&item2);
		item3.link.Unlink(&item3);
		BOOST_ASSERT(list.getSize() == 0);
	}
};

BOOST_FIXTURE_TEST_CASE(list_insert, insert_fixture)
{
	list.Insert(&item1);
	BOOST_REQUIRE_EQUAL(list.getSize(), 1);
	BOOST_REQUIRE_EQUAL(list.getFirst(), &item1);
	
	list.Insert(&item2);
	BOOST_REQUIRE_EQUAL(list.getSize(), 2);
	BOOST_REQUIRE_EQUAL(list.getFirst(), &item2);

	list.Insert(&item3);
	BOOST_REQUIRE_EQUAL(list.getSize(), 3);
	BOOST_REQUIRE_EQUAL(list.getFirst(), &item3);
}

BOOST_FIXTURE_TEST_CASE(list_insert_before, insert_fixture)
{
	list.Insert(&item1);
	
	list.InsertBefore(&item2, &item1);
	BOOST_REQUIRE_EQUAL(list.getSize(), 2);
	BOOST_REQUIRE_EQUAL(list.getFirst(), &item2);

	list.InsertBefore(&item3, &item1);
	BOOST_REQUIRE_EQUAL(list.getSize(), 3);
	BOOST_REQUIRE_EQUAL(list.getFirst(), &item2);
	BOOST_REQUIRE_EQUAL(list.getNext(&item2), &item3);
}

BOOST_FIXTURE_TEST_CASE(list_insert_after, insert_fixture)
{
	list.Insert(&item1);
	
	list.InsertAfter(&item2, &item1);
	BOOST_REQUIRE_EQUAL(list.getSize(), 2);
	BOOST_REQUIRE_EQUAL(list.getFirst(), &item1);
	BOOST_REQUIRE_EQUAL(list.getNext(&item1), &item2);

	list.InsertAfter(&item3, &item1);
	BOOST_REQUIRE_EQUAL(list.getSize(), 3);
	BOOST_REQUIRE_EQUAL(list.getFirst(), &item1);
	BOOST_REQUIRE_EQUAL(list.getNext(&item1), &item3);
}

struct remove_fixture {
	List<testItem> list;
	testItem item1;
	testItem item2;
	testItem item3;

	remove_fixture() : list(&testItem::link), item1(), item2(), item3() {
		list.Insert(&item3);
		list.Insert(&item2);
		list.Insert(&item1);
	}

	virtual ~remove_fixture() {
		BOOST_ASSERT(list.getSize() == 0);
	}
};

BOOST_FIXTURE_TEST_CASE(list_remove_first, remove_fixture)
{
	BOOST_REQUIRE_EQUAL(list.getFirst(), &item1);
	list.Remove(&item1);
	BOOST_REQUIRE_EQUAL(list.getFirst(), &item2);
	list.Remove(&item2);
	BOOST_REQUIRE_EQUAL(list.getFirst(), &item3);
	list.Remove(&item3);
	BOOST_REQUIRE(list.getFirst() == 0);
}

BOOST_FIXTURE_TEST_CASE(list_remove_last, remove_fixture)
{
	list.Remove(&item3);
	BOOST_REQUIRE(list.getNext(&item2) == 0);
	list.Remove(&item2);
	BOOST_REQUIRE(list.getNext(&item1) == 0);
	list.Remove(&item1);
	BOOST_REQUIRE(list.getFirst() == 0);
}

BOOST_FIXTURE_TEST_CASE(list_remove_middle, remove_fixture)
{
	list.Remove(&item2);
	BOOST_REQUIRE_EQUAL(list.getNext(&item1), &item3);
	list.Remove(&item1);
	list.Remove(&item3);
}

BOOST_AUTO_TEST_SUITE_END()
