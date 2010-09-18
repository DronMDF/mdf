//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>

#include "StubLocal.h"
#include "Arch.h"
#include "Device.h"

// -----------------------------------------------------------------------------
// Программируемый таймер i8253/i8254

// http://www.cs.mun.ca/~paul/cs3725/material/web/notes/node31.html
//	Смешно, но эта инструкция сильно помогла.
// http://en.wikipedia.org/wiki/Intel_8253

static
const device_t i825x_device = {
	.io_base = 0x40,
	.io_last = 0x4f,
};

enum {
	I8254_COUNTER0_REG = 0,
	I8254_COUNTER1_REG = 1,
	I8254_COUNTER2_REG = 2,

	I8254_CONTROL_REG = 3,
};

enum {
	I8254_BCD = 1,

	I8254_MODE_0 = 0 << 1,
	I8254_MODE_1 = 1 << 1,
	I8254_MODE_2 = 2 << 1,
	I8254_MODE_3 = 3 << 1,
	I8254_MODE_4 = 4 << 1,
	I8254_MODE_5 = 5 << 1,

	I8254_LATCH = 0,
	I8254_RW0 = 1 << 4,
	I8254_RW1 = 1 << 5,
	I8254_RW_ALL = I8254_RW0 | I8254_RW1,

	I8254_COUNTER0 = 0 << 6,
	I8254_COUNTER1 = 1 << 6,
	I8254_COUNTER2 = 2 << 6,
};

enum {
	I8254_STATUS_OUTPUT	= 1 << 7,
};

static
const uint32_t i8254_frequency = 1193182;

static
void StubI8254InitCounter0 (uint32_t hz)
{
	const uint8_t mode = I8254_MODE_3 | I8254_RW_ALL | I8254_COUNTER0;
	const uint32_t count = i8254_frequency / hz;
	STUB_ASSERT (count > 0xffff, "Minimim 19Hz!");

	StubDeviceWriteB (&i825x_device, I8254_CONTROL_REG, mode);
	StubDeviceWriteB (&i825x_device, I8254_COUNTER0_REG, count & 0xff);
	StubDeviceWriteB (&i825x_device, I8254_COUNTER0_REG, (count >> 8) & 0xff);
}

// Этот каунтер программируем микросекундами.
static
void StubI8254InitCounter2 (uint32_t count)
{
	const uint8_t mode = I8254_MODE_0 | I8254_RW_ALL | I8254_COUNTER2;

	StubDeviceWriteB (&i825x_device, I8254_CONTROL_REG, mode);
	StubDeviceWriteB (&i825x_device, I8254_COUNTER2_REG, count & 0xff);
	StubDeviceWriteB (&i825x_device, I8254_COUNTER2_REG, (count >> 8) & 0xff);
}

static
uint32_t StubI8254ReadCounter2 ()
{
	const uint8_t mode = I8254_LATCH | I8254_COUNTER2;

	StubDeviceWriteB (&i825x_device, I8254_CONTROL_REG, mode);
	const uint32_t low = StubDeviceReadB (&i825x_device, I8254_COUNTER2_REG);
	const uint32_t hi = StubDeviceReadB (&i825x_device, I8254_COUNTER2_REG);

	return (hi << 8) | low;
}

static tick_t stubClock = 0;

void StubClockHandler()
{
	stubClock++;

	// Переключаемся на кого нибудь (или не переключаемся).
	const Task *task = StubGetCurrentTask();
	CoreWait(task, 0, 0, 0);
}

tick_t StubGetCurrentClock()
{
	return stubClock;
}

// Микросекунда - это 1/1000000 секунды.
// Сокращённо обозначается как µs или us.

// Эта функция несколько завышает таймаут, но сойдет для начала,
// главное чтобы не занижала.
void StubMicroSleep (uint32_t us)
{
	STUB_ASSERT (us > 54925, "Maximal micro timeout is 54925us = 0,055s");

	// Используем для этого второй канал таймера.
	StubI8254InitCounter2 (65535);

	const uint32_t lowcount = (uint32_t)(65535 - ((uint64_t)i8254_frequency * us) / 1000000);

	while (StubI8254ReadCounter2() > lowcount);
}

void __init__ StubTimeInit ()
{
	// Понизим пока частоту до сотни, а то bochs не справляется.
	StubI8254InitCounter0(100);
	StubInterruptUnmask(0);

	CorePrint ("Timer initialized...\n");
}

