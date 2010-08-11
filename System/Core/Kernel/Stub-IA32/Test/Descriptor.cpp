//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

extern "C" {
#include "../Descriptor.h"
}

BOOST_AUTO_TEST_SUITE(suiteDescriptor)

BOOST_AUTO_TEST_CASE(testBase)
{
	descriptor_t desc = StubGenerateSegmentDescriptor(0xAABBCCDD, 1, 0);
	BOOST_REQUIRE_EQUAL(desc.raw, 0xAA0080BBCCDD0000LL);
}

BOOST_AUTO_TEST_CASE(testSize)
{
	descriptor_t desc = StubGenerateSegmentDescriptor(0, 0x100000, 0);
	BOOST_REQUIRE_EQUAL(desc.raw, 0x000f80000000ffffLL);
}

BOOST_AUTO_TEST_CASE(testGranularitySize)
{
	//  Заодно проверяем округление в большую сторону
	descriptor_t desc = StubGenerateSegmentDescriptor(0, 0x10001f00, 0);
	BOOST_REQUIRE_EQUAL(desc.raw, 0x0081800000000001LL);
}

BOOST_AUTO_TEST_CASE(testFullSize)
{
	// Специальный случай, когда размер равен нулю
	descriptor_t desc = StubGenerateSegmentDescriptor(0, 0, 0);
	BOOST_REQUIRE_EQUAL(desc.raw, 0x008f80000000ffffLL);
}

BOOST_AUTO_TEST_CASE(testFlags)
{
	// PRESENT и GRANULARITY ставится автоматически
	descriptor_t desc = StubGenerateSegmentDescriptor(0, 1, 0x77F);
	BOOST_REQUIRE_EQUAL(desc.raw, 0x0070ff0000000000LL);
}

BOOST_AUTO_TEST_CASE(testGetBase)
{
	const laddr_t  base = 0x12345678;;
	descriptor_t desc = StubGenerateSegmentDescriptor(base, 0, 0);
	BOOST_REQUIRE_EQUAL(StubDescriptorGetBase(desc), base);
}

BOOST_AUTO_TEST_CASE(testGetSize)
{
	const size_t size = 0xFEDCB;
	descriptor_t desc = StubGenerateSegmentDescriptor(0, size, 0);
	BOOST_REQUIRE_EQUAL(StubDescriptorGetSize(desc), size);
}

BOOST_AUTO_TEST_CASE(testGetSizeGranularity)
{
	const size_t size = 0xFEDCB000;
	descriptor_t desc = StubGenerateSegmentDescriptor(0, size, 0);
	BOOST_REQUIRE_EQUAL(StubDescriptorGetSize(desc), size);
}

BOOST_AUTO_TEST_SUITE_END()
