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
#include "../Resource.h"
#include "../ResourceInstance.h"
#include "../Region.h"
#include "../Thread.h"
#include "../Process.h"

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
		: ResourceRegion(0, PAGE_SIZE, 0),
			m_first(first), m_last(last) {}
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
	region->Register();
	process.Attach(region, 0, PAGE_SIZE);	// USER_MEMORY_BASE + PAGE_SIZE

	char data[PAGE_SIZE];
	fill_random(data, PAGE_SIZE);

	BOOST_REQUIRE(process.copyIn(USER_MEMORY_BASE + PAGE_SIZE, data, PAGE_SIZE));
}

// TODO: Еще проверить три рядомстоящих региона,
// А так же сбои по отсутствию инстанций или по дыркам между регионами.

BOOST_AUTO_TEST_SUITE_END()
