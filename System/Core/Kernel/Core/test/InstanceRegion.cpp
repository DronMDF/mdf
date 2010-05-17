//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/Kernel.h"
#include "../include/ResourceRegion.h"
#include "../include/InstanceRegion.h"

using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteInstanceRegion)

BOOST_AUTO_TEST_CASE(testPageFault)
{
	const size_t parent_size = 10000;
	const size_t parent_window_offset = 2000;
	const size_t parent_window_size = 5000;
	const size_t child_size = 20000;
	
	Resource *region = new ResourceRegion(parent_size, 
				RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE);
				
	// Необходимость позиционировать регион без биндинга - пока не определилась
	InstanceRegion instance(region, RESOURCE_ACCESS_READ, 
		parent_window_offset, parent_window_size, child_size - parent_size);
	
	// Инстанция должна ограничить права чтением
	uint32_t access = RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE;
	BOOST_WARN(instance.PageFault(child_size - parent_size + parent_window_offset, &access));
	BOOST_WARN_EQUAL(access, RESOURCE_ACCESS_READ);
}

BOOST_AUTO_TEST_SUITE_END()
