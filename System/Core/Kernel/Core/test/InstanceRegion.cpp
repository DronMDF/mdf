//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/InstanceRegion.h"

using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteInstanceRegion)

BOOST_AUTO_TEST_CASE(testCreation)
{
	InstanceRegion instance(0, 0, 0, 0, 0);
}

BOOST_AUTO_TEST_SUITE_END()
