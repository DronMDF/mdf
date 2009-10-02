//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>

#include "StubLocal.h"
#include "Arch.h"

// -----------------------------------------------------------------------------
// i8259 intel interrupt controller
// TODO: Надо выделить код отдельного контроллера, это упростит разруливание.

enum I8259_IO {
	I8259_ISR	= 0,
	I8259_IMR	= 1,
};

enum I8259_CONTROLLERS {
	I8259_MASTER	= 0x20,
	I8259_SLAVE	= 0xa0,
};

enum I8259_COMMANDS {
	I8259_COMMAND_ICW1	= 0x11,
	I8259_COMMAND_EOI	= 0x20,
	I8259_COMMAND_CEOI	= 0x60,
};

static
void StubI8259WriteCommand (const int base, const unsigned char value)
{
	outbyte (base + I8259_ISR, value);
	iowait ();
}

static
void StubI8259WriteControl (const int base, const unsigned char value)
{
	outbyte (base + I8259_IMR, value);
	iowait();
}

static
void StubI8259WriteMask (const int base, const unsigned char value)
{
	outbyte (base + I8259_IMR, value);
}

static
unsigned char StubI8259ReadMask (const int base)
{
	return inbyte (base + I8259_IMR);
}

// TODO: Этот код надо разрулить по отдельным контроллерам.
static
void __init__ StubI8259Init (void)
{
	// Master controller
	StubI8259WriteCommand (I8259_MASTER, I8259_COMMAND_ICW1);
	StubI8259WriteControl (I8259_MASTER, 32);	// Базовый вектор
	StubI8259WriteControl (I8259_MASTER, 4);
	StubI8259WriteControl (I8259_MASTER, 1);

	// Slave controller
	StubI8259WriteCommand (I8259_SLAVE, I8259_COMMAND_ICW1);
	StubI8259WriteControl (I8259_SLAVE, 40);	// Базовый вектор
	StubI8259WriteControl (I8259_SLAVE, 2);
	StubI8259WriteControl (I8259_SLAVE, 1);

	StubI8259WriteMask (I8259_MASTER, 0xff);	// все замаскированы.
	StubI8259WriteMask (I8259_SLAVE, 0xff);		// все замаскированы.
}

static
void StubI8259Mask (int irq)
{
	if (irq < 8) {
		uint8_t mask = StubI8259ReadMask (I8259_MASTER);
		mask |= 1 << irq;
		StubI8259WriteMask (I8259_MASTER, mask);
	} else {
		STUB_ASSERT (irq > 15, "Very big irq");
		uint8_t mask = StubI8259ReadMask (I8259_SLAVE);
		mask |= 1 << (irq - 8);
		StubI8259WriteMask (I8259_SLAVE, mask);
	}
}

static
void StubI8259Unmask (int irq)
{
	CorePrint ("Unmask IRQ #%u\n", irq);
	if (irq < 8) {
		uint8_t mask = StubI8259ReadMask (I8259_MASTER);
		mask &= ~(1 << irq);
		StubI8259WriteMask (I8259_MASTER, mask);
	} else {
		STUB_ASSERT (irq > 15, "Very big irq");
		uint8_t mask = StubI8259ReadMask (I8259_SLAVE);
		mask &= ~(1 << (irq - 8));
		StubI8259WriteMask (I8259_SLAVE, mask);
	}
}

static
bool StubI8259IsMasked (int irq)
{
	if (irq < 8) {
		uint8_t mask = StubI8259ReadMask (I8259_MASTER);
		return (mask & (1 << irq)) != 0;
	}

	STUB_ASSERT (irq > 15, "Very big irq");
	if (StubI8259IsMasked (2))	// Каскадное прерывание
		return true;

	uint8_t mask = StubI8259ReadMask (I8259_SLAVE);
	return (mask & (1 << (irq - 8))) != 0;
}

static
bool StubI8259IsServiced (int irq)
{
	uint16_t irr = 0;

	if (irq < 8) {
		StubI8259WriteCommand (I8259_MASTER, 0x0b);
		irr = inbyte (I8259_MASTER + I8259_ISR);
		StubI8259WriteCommand (I8259_MASTER, 0x0a);
	} else {
		STUB_ASSERT (irq > 15, "Very big irq");

		StubI8259WriteCommand (I8259_SLAVE, 0x0b);
		irr = inbyte (I8259_SLAVE + I8259_ISR) << 8;
		StubI8259WriteCommand (I8259_SLAVE, 0x0a);
	}

	return (irr & (1 << irq)) != 0;
}

static
void StubI8259Acknowledge (int irq)
{
	// TODO: EOI лучше посылать конкретный.
	// У него код 0x60 + младший номер прерывания (0...7)

	if (irq < 8) {
		StubI8259WriteCommand (I8259_MASTER, I8259_COMMAND_CEOI + irq);
	} else {
		STUB_ASSERT (irq > 15, "Very big irq");
		StubI8259WriteCommand (I8259_SLAVE, I8259_COMMAND_CEOI + irq - 8);
		StubI8259WriteCommand (I8259_MASTER, I8259_COMMAND_CEOI + 2);
	}
}

// -----------------------------------------------------------------------------
// Интерфейсные процедуры.
void __init__ StubInterruptControllerInit (void)
{
	StubI8259Init ();

	CorePrint ("Interrupt controller initialized...\n");

	// Настроив таймер можно попробовать продетектить частоту проца...
	// Хотя настройке таймера да и измерению частоты тут не место...
// 	outbyte (0x43, 0x36);				// PIT_MODE
//         outbyte (0x40, (1193180 / 1000) & 0xff);	// PIT_CH0
//         outbyte (0x40, ((1193180 / 1000) >> 8) & 0xff);	// PIT_CH0
}

void StubInterruptMask (int irq)
{
	StubI8259Mask (irq);
}

void StubInterruptUnmask (int irq)
{
	StubI8259Unmask (irq);
}

bool StubInterruptIsMasked (int irq)
{
	if (!StubI8259IsMasked (irq))
		return false;

	// Может быть замаскировано, но прерывание всеравно валидное
	if (StubI8259IsServiced (irq))
		return false;

	return true;
}

void StubInterruptAcknowledge (int irq)
{
	StubI8259Acknowledge (irq);
}
