//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

typedef struct {
	uint32_t io_base;
	uint32_t io_last;
} device_t;

static inline
void StubDeviceWriteB (const device_t *dev, uint16_t io, uint8_t value)
{
	STUB_ASSERT (dev->io_base > dev->io_last, "Invalid device parameters");
	STUB_ASSERT (io > dev->io_last - dev->io_base, "Invalid device port");

	__asm__ __volatile__ (
		"outb %b0, %w1" : :
		"a"(value), "Nd"(dev->io_base + io));
}

static inline
uint8_t StubDeviceReadB (const device_t *dev, uint16_t io)
{
	STUB_ASSERT (dev->io_base > dev->io_last, "Invalid device parameters");
	STUB_ASSERT (io > dev->io_last - dev->io_base, "Invalid device port");

        uint8_t value = 0;
        __asm__ __volatile__ (
		"inb %w1, %b0"
		: "=a"(value)
		: "Nd" (dev->io_base + io));

        return value & 0xff;
}

static inline
void __deprecated__ StubDeviceDelay ()
{
	device_t delay_dev = {
		.io_base = 0x80,
		.io_last = 0x80,
	};

	StubDeviceWriteB (&delay_dev, 0, 0);
}
