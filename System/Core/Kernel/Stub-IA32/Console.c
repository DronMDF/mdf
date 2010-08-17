//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>

#include "StubLocal.h"
#include "Device.h"
#include "Utils.h"

// -----------------------------------------------------------------------------
// Драйвер клавиатуры
static
const device_t kbd_device = {
	.io_base = 0x60,
	.io_last = 0x6f,
};

static
unsigned char StubKeyboardRead (void)
{
	while (!isSet(StubDeviceReadB(&kbd_device, 4), 1));
	return StubDeviceReadB(&kbd_device, 0);
}

static
void StubKeyboardWrite (const uint16_t port, const unsigned char val)
{
	while (isSet(StubDeviceReadB(&kbd_device, 4), 2));
	StubDeviceWriteB(&kbd_device, port, val);
}

static
int StubKeyboardGetChar (void)
{
	StubKeyboardWrite(4, 0x60);
	StubKeyboardWrite(0, 0x60);

	while (true) {
		StubDeviceWriteB(&kbd_device, 1, StubDeviceReadB(&kbd_device, 1) | 0x80);
		StubDeviceWriteB(&kbd_device, 1, StubDeviceReadB(&kbd_device, 1) & 0x7f);

		int code = StubKeyboardRead ();

		if ((code & 0x80) != 0)
			continue;

		switch (code) {
			case 0x01:
				// ESC вызывает рестарт
				StubDeviceWriteB(&kbd_device, 4, 0xfe);
				break;

			case 0x20:
				// Кнопка 'D' Вызывает отладчик боша.
				while (1) {
					const device_t bochs_device = {
						.io_base = 0x8900,
						.io_last = 0x890f,
					};

					StubDeviceWriteB(&bochs_device, 0, 'D');
				}
				break;

			case 0x13:
				return 'R';

			default:
				//CorePrint ("Code 0x%02x is not handle\n", code);
				break;
		}
	}

	return 0;
}

// -----------------------------------------------------------------------------
// xGA консоль
static
const device_t xga_device = {
	.io_base = 0x3d0,
	.io_last = 0x3df,
};

static
unsigned long StubXGAConsoleReadPos(void)
{
	unsigned long p = 0;

	StubDeviceWriteB(&xga_device, 4, 14);
	p = StubDeviceReadB(&xga_device, 5);
	p <<= 8;

	StubDeviceWriteB(&xga_device, 4, 15);
	p |= StubDeviceReadB(&xga_device, 5);

	return p;
}

static
void StubXGAConsoleSetPos(const unsigned long pos)
{
	StubDeviceWriteB(&xga_device, 4, 14);
	StubDeviceWriteB(&xga_device, 5, (pos >> 8) & 0xff);
	StubDeviceWriteB(&xga_device, 4, 15);
	StubDeviceWriteB(&xga_device, 5, pos & 0xff);
}

static
void StubXGAConsolePrintChar(const int c)
{
	uint16_t *video = (uint16_t *)VIDEO0_PAGE;
	unsigned long pos = StubXGAConsoleReadPos();

	switch (c) {
		case '\n':
			pos = round_up (pos + 1, 80);
			break;
		case '\r':
			pos = round (pos, 80);
			break;
		case '\t':
			pos = round_up (pos + 1, 8);
			break;
		default:
			video[pos++] = (uint16_t)(c | 0x700);
			break;
	}

	if (pos >= 80 * 25) {
		// Скролл
		StubMemoryCopy (video, video + 80, 80 * 24 * 2);
		StubMemoryClear (video + 80 * 24, 80 * 2);
		pos -= 80;
	}

	StubXGAConsoleSetPos (pos);
}

// -----------------------------------------------------------------------------
// Серийная консоль
static
const device_t serial_device = {
	.io_base = 0x3f8,
	.io_last = 0x3ff,
};

// Нагло позаимствовал из xskernel by SadKo (www.xskernel.org)

#define COM_REGISTER_THR            0x0000      /* Transmission Register                */
#define COM_REGISTER_RBR            0x0000      /* Baud Rate Register                   */
#define COM_REGISTER_IER            0x0001      /* Interrupts Enable Register           */
#define COM_REGISTER_IIR            0x0002      /* Interrupt Identification Register    */
#define COM_REGISTER_FCR            0x0002      /* FIFO Control Register                */
#define COM_REGISTER_LCR            0x0003      /* Line Control Register                */
#define COM_REGISTER_MCR            0x0004      /* Modem Control Register               */
#define COM_REGISTER_LSR            0x0005      /* Line State Register                  */
#define COM_REGISTER_MSR            0x0006      /* Modem State Register                 */
#define COM_REGISTER_FREE           0x0007      /* Free Register                        */

#define COM_LCR_BR                  0x80        /* THR And IER Are Frequency Divisor    */
#define COM_LCR_BYTELEN_8           0x03        /* 8-Bit Byte Length                    */
#define COM_LCR_PARITY_NO_CONTROL   0x04        /* No Parity Control                    */

#define COM_BR_115200               0x0001      /* Baud Rate 115200                     */

#define COM_IER_MSC                 0x08        /* Modem State Change                   */
#define COM_IER_BREAK               0x04        /* BREAK State Or Error                 */
#define COM_IER_TX                  0x02        /* Transmission Buffer Is Free          */
#define COM_IER_RX                  0x01        /* Received Data                        */

#define COM_FCR_TX_14               0xc0        /* TX Buffer: 14 Bytes                  */
#define COM_FCR_TXCLEAR             0x04        /* TX Buffer Clear                      */
#define COM_FCR_RXCLEAR             0x02        /* RX Buffer Clear                      */
#define COM_FCR_FIFO                0x01        /* Enable FIFO                          */

#define COM_MCR_OUT2                0x08        /* Line OUT2                            */
#define COM_MCR_DTR                 0x01        /* DTR                                  */

#define COM_LSR_TXREADY             0x20        /* THX Register Is Ready To Write       */

static
void StubSerialConsoleInit()
{
	StubDeviceWriteB(&serial_device, COM_REGISTER_LCR, COM_LCR_BR);
	// Может быть WriteW?
	StubDeviceWriteB(&serial_device, COM_REGISTER_RBR + 1, (COM_BR_115200 >> 8) & 0xff);
	StubDeviceWriteB(&serial_device, COM_REGISTER_RBR, COM_BR_115200 & 0xff);

	StubDeviceWriteB(&serial_device, COM_REGISTER_LCR,
		(COM_LCR_BYTELEN_8 | COM_LCR_PARITY_NO_CONTROL) & 0x7f);

	StubDeviceWriteB(&serial_device, COM_REGISTER_IER,
		COM_IER_MSC | COM_IER_BREAK | COM_IER_TX | COM_IER_RX);

	StubDeviceWriteB(&serial_device, COM_REGISTER_FCR,
		COM_FCR_TX_14 | COM_FCR_TXCLEAR | COM_FCR_RXCLEAR | COM_FCR_FIFO);

	StubDeviceWriteB(&serial_device, COM_REGISTER_MCR, COM_MCR_OUT2 | COM_MCR_DTR);
	StubDeviceWriteB(&serial_device, COM_REGISTER_FREE, 0);
}

static
void StubSerialConsolePrintChar(const int c)
{
	while (!isSet(StubDeviceReadB(&serial_device, COM_REGISTER_LSR), COM_LSR_TXREADY));
	StubDeviceWriteB(&serial_device, COM_REGISTER_THR, (uint8_t)c);
	if (c == '\n') StubSerialConsolePrintChar('\r');
}

// -----------------------------------------------------------------------------
// Стабовая прослойка вывода символов.

// TODO: ПО умолчанию стоит прикрутить специальный консольный враппер,
// который будет аккумулировать инфу.

enum CONSOLE_TYPES {
	CONSOLE_NONE	= 0x00000000,
	CONSOLE_XGA	= 0x00000001,
	CONSOLE_SERIAL	= 0x00000002,
};

static
int consoles = CONSOLE_NONE;

bool StubSetConsole(const char * const type)
{
	if (type == nullptr) {
		if (consoles == CONSOLE_NONE)
			consoles |= CONSOLE_XGA;
		return true;
	}

	if (StubStringEqual (type, "xga", 3)) {
		CorePrint ("Activate xGA console\n");
		consoles |= CONSOLE_XGA;
	} else if (StubStringEqual (type, "serial", 6)) {
		CorePrint ("Activate serial console\n");
		consoles |= CONSOLE_SERIAL;

		StubSerialConsoleInit();
	} else {
		CorePrint ("Unsupported console type\n");
		return false;
	}

	return true;
}

void StubPrintChar (const int c)
{
	if (isSet(consoles, CONSOLE_SERIAL)) {
		StubSerialConsolePrintChar(c);
	} else {
		// Вывод по умолчанию.
		// if (isSet(consoles, CONSOLE_XGA)) {
		StubXGAConsolePrintChar(c);
	}
}

int StubGetChar (void)
{
	return StubKeyboardGetChar();
}
