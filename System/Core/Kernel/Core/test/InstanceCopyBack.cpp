//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/InstanceCopyBack.h"
#include "testThread.h"
#include "TestHelpers.h"

using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteInstanceCopyBack)

// Временно
BOOST_AUTO_TEST_CASE(testCopyBack)
{
	struct inThread : public testThread, private visit_mock {
		enum { DST = 111, SIZE = 222 };
		bool copyIn(laddr_t dst, const void *, size_t size) {
			BOOST_REQUIRE_EQUAL(dst, laddr_t(DST));
			BOOST_REQUIRE_EQUAL(size, size_t(SIZE));
			visit();
			return true;
		}
	} *thread = new inThread;
	
	InstanceCopyBack instance(thread, inThread::DST);
	// При копировании указываем позицию буфера У нас и его размер (он мог измениться)
	instance.copyIn(0, inThread::SIZE);
	// После деструкции инстанции нить умрет сама.
}

BOOST_AUTO_TEST_SUITE_END()
