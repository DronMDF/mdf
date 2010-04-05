//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <iostream>
#include <boost/format.hpp>
#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../Core.h"
#include "../Kernel.h"
#include "../List.h"
#include "../Memory.h"
#include "../Instance.h"

#include "../Resource.h"
#include "../ResourceRegion.h"
#include "../ResourceProcess.h"
#include "../Thread.h"

#include "testProcess.h"
#include "TestHelpers.h"

using namespace std;
using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(process)

BOOST_AUTO_TEST_CASE(detach)
{
	// Удаление ресурсов всегда происходит через инстанцию. За исключением
	// процессов, с которыми немного иначе. С пассивными ресурсами все
	// просто, а нити удаляются через процесс, ссылка на который в них
	// хранится. Удаление происходит через вызов метода Detach. Процесс
	// обнаруживает инстанцию и удаляет ее. Ресур, обрнаружив удаление
	// последней инстанции удаляется сам.

	// Для тестирования детача необходимо к процессу подцепить ресурс через
	// инстанцию.

	bool deleted = false;

	testProcess process;

	class testResource : public Resource {
	private:
		bool *m_delete;

		testResource(const testResource &);
		testResource &operator =(const testResource &);
	public:
		testResource(bool *d) : Resource(), m_delete(d) {}
		~testResource() { *m_delete = true; }
	} *resource = new testResource(&deleted);

	resource->Register();
	process.Attach(resource, 0, 0);
	process.Detach(resource);
	BOOST_REQUIRE(deleted);
}

struct testRegion : public ResourceRegion, private visit_mock {
	bool m_first, m_last;
	testRegion(bool first, bool last)
		: ResourceRegion(PAGE_SIZE, 0), m_first(first), m_last(last) 
	{ 
		Register(); 
	}
	virtual bool copyIn(offset_t offset, const void *src, size_t size) {
		visit();
		if (m_first) BOOST_REQUIRE(offset == 0 && size >= PAGE_SIZE / 2);
		if (m_last) BOOST_REQUIRE(offset <= PAGE_SIZE / 2 && offset + size == PAGE_SIZE);
		return ResourceRegion::copyIn(offset, src, size);
	}
};

BOOST_AUTO_TEST_CASE(testCopyIn)
{
	testProcess process;
	
	ResourceRegion *region = new testRegion(true, true);
	process.Attach(region, 0, PAGE_SIZE);	// USER_MEMORY_BASE + PAGE_SIZE

	char data[PAGE_SIZE];
	generate(data, data + PAGE_SIZE, rand);

	BOOST_REQUIRE(process.copyIn(USER_MEMORY_BASE + PAGE_SIZE, data, PAGE_SIZE));
}

BOOST_AUTO_TEST_CASE(testCopyInInterReg)
{
	testProcess process;
	
	ResourceRegion *region1 = new testRegion(false, true);
	ResourceRegion *region2 = new testRegion(true, true);
	ResourceRegion *region3 = new testRegion(true, false);
	
	process.Attach(region1, 0, PAGE_SIZE);	// from USER_MEMORY_BASE
	process.Attach(region2, 0, PAGE_SIZE + PAGE_SIZE);
	process.Attach(region3, 0, PAGE_SIZE + PAGE_SIZE * 2);

	char data[PAGE_SIZE * 2];
	generate(data, data + PAGE_SIZE * 2, rand);

	BOOST_REQUIRE(process.copyIn(USER_MEMORY_BASE + PAGE_SIZE + PAGE_SIZE / 2, 
				     data, PAGE_SIZE * 2));
}

// TODO: А так же сбои по отсутствию инстанций или по дыркам между регионами.
struct testHoleRegion : public ResourceRegion {
	testHoleRegion(size_t size) 
		: ResourceRegion(size, 0) 
	{ 
		Register(); 
	}
	virtual bool copyIn(offset_t, const void *, size_t) {
		throw std::runtime_error("No one call");
	}
};

BOOST_AUTO_TEST_CASE(testCopyInHoleReg)
{
	testProcess process;
	
	ResourceRegion *region1 = new testHoleRegion(PAGE_SIZE - 1);
	ResourceRegion *region2 = new testHoleRegion(PAGE_SIZE);
	
	process.Attach(region1, 0, PAGE_SIZE);	// from USER_MEMORY_BASE
	// Здесь между регионамы - дырка... размером один байт
	process.Attach(region2, 0, PAGE_SIZE + PAGE_SIZE);

	char data[PAGE_SIZE];
	generate(data, data + PAGE_SIZE, rand);

	BOOST_REQUIRE(!process.copyIn(USER_MEMORY_BASE + PAGE_SIZE + PAGE_SIZE / 2, 
				     data, PAGE_SIZE));
}

BOOST_AUTO_TEST_SUITE_END()
