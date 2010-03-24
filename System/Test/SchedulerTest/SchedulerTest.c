//
// Copyright (c) 2000-2009 Andrey Valyaev (dron@infosec.ru)
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#ifdef TEST
	#include <limits.h>

	#include <iostream>

	#define BOOST_TEST_MAIN
	#define BOOST_TEST_DYN_LINK
	#include <boost/test/unit_test.hpp>

	using namespace std;
#else
	#include <MDF/Types.h>
	#include <MDF/Kernel.h>
	#include <MDF/KernelImp.h>

	#include <stdio.h>
#endif

static
int square(int value)
{
	int s = 0;
	while (value > 0) {
		value -= s * 2 + 1;
		s++;
	}
	return s;
}

static
int calculatePriority (int pos)
{
	const int dx = pos % 160 - 80;	// -80 ... 80
	const int dy = (pos / 160 - 12) * 4; // -48 ... 48
	return 100 - square(dx * dx + dy * dy);
}

#ifdef TEST

BOOST_AUTO_TEST_CASE(test_square)
{
	BOOST_CHECK(square(1) == 1);
	BOOST_CHECK(square(4) == 2);
	BOOST_CHECK(square(9) == 3);
	BOOST_CHECK(square(443556) == 666);
}

BOOST_AUTO_TEST_CASE(test_priority)
{
	BOOST_CHECK (calculatePriority(162) == calculatePriority(318));
	BOOST_CHECK (calculatePriority(160) < calculatePriority(12 * 160 + 80));
}

#else // TEST undefined

char id[] = "MDFVER: System-Test/SchedulerTest-" VERSION;

static volatile char *vbuf = NULL;

static
volatile void *getVideoMemory ()
{
	id_t rid = INVALID_ID;

	const struct KernelCreateRegionParam cpar = {
		.size = 4000,
		.access = RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE,
	};

	if (KernelCreate(RESOURCE_TYPE_REGION, &cpar, sizeof(cpar), &rid) != SUCCESS)
		return NULL;

	if (rid == INVALID_ID)
		return NULL;

	// Биндим на физическую видеопамять
	const struct KernelModifyRegionBindParam mpar = {
		.id = 0,
		.offset = 0xb8000,
		.size = 4000,
		.shift = 0,
	};

	if (KernelModify(rid, RESOURCE_MODIFY_REGION_PHYSICALBIND, &mpar, sizeof(mpar)) != SUCCESS) {
		KernelDetach(rid, 0);
		return NULL;
	}

	laddr_t addr = 0;
	if (KernelModify(rid, RESOURCE_MODIFY_REGION_MAP, &addr, sizeof(laddr_t)) != SUCCESS) {
		KernelDetach(rid, 0);
		return NULL;
	}

	size_t addr_size = sizeof(laddr_t);
	if (KernelInfo(rid, RESOURCE_INFO_REGION_INSTANCE_ADDR, &addr, &addr_size) != SUCCESS) {
		KernelDetach(rid, 0);
		return NULL;
	}

	return (volatile void *)addr;
}

static
id_t getThreadId()
{
	id_t thread = INVALID_ID;
	size_t idsize = sizeof (id_t);

	KernelInfo(0, RESOURCE_INFO_THREAD_CURRENT, &thread, &idsize);
	return thread;
}

static
void setThreadPriority(uint32_t priority)
{
	id_t thread = getThreadId();
	KernelModify(thread, RESOURCE_MODIFY_THREAD_PRIORITY, &priority, sizeof(uint32_t));
}

void
BlinkThread (id_t process __attribute__((unused)),
	void *param, size_t size __attribute__((unused)),
	uint32_t flags __attribute__((unused)))
{
	int pos = *(int *)param;

	static const char code[] = { 0xbc, 0xc8, 0xc9, 0xbb, 0xbe, 0xc0, 0xda, 0xb8};

	setThreadPriority(calculatePriority(pos));

	vbuf[pos + 1] = 0xf;

	for (int c = 0; ; c++) {
		vbuf[pos] = code[c % 8];
		KernelWait(0, 0, 0);
	}
}

int main (int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	vbuf = getVideoMemory();
	if (vbuf == NULL) return -1;

	for (int i = 80 * 2; i < 80 * 25 * 2; i += 2) {
	//for (int i = 80 * 2; i < 90 * 2; i += 2) {	// 10 нитей.
		id_t nt = INVALID_ID;

		struct KernelCreateThreadParam tp = {
			.entry = (laddr_t)BlinkThread,
		};

		KernelCreate(RESOURCE_TYPE_THREAD, &tp, sizeof(tp), &nt);
		if (nt != INVALID_ID) {
			KernelCall(nt, &i, sizeof(int), RESOURCE_CALL_ASYNC |
				RESOURCE_CALL_COPY | RESOURCE_CALL_READONLY);
		}
	}

	return 0;
}

#endif // TEST
