
extern "C" {
#include "../Descriptor.h"
}

BOOST_AUTO_TEST_SUITE(suiteDescriptor)

BOOST_AUTO_TEST_CASE(testBase)
{
	descriptor_t desc = StubGenerateSegmentDescriptor(0xAABBCCDD, 1, 0);
	BOOST_REQUIRE_EQUAL(desc.raw, 0xAA0000BBCCDD0000LL);
}

BOOST_AUTO_TEST_CASE(testSize)
{
	descriptor_t desc = StubGenerateSegmentDescriptor(0, 0x100000, 0);
	BOOST_REQUIRE_EQUAL(desc.raw, 0x000f00000000ffffLL);

	// TODO: проверить гранулирование
}

// TODO: Протестировать флаги


BOOST_AUTO_TEST_SUITE_END()
