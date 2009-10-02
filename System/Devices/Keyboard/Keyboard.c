//
// Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
// All rights reserved.
//
// Создано: Чтв Окт 27 22:06:43 2005
//

// Материалы для работы
// Где бы даташит найти???


#include <MDF/Kernel2.h>
#include <MDF/IOPorts.h>
#include <MDF/Locks.h>

#include <MDF/Namer.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

void KeyboardService (handle tid, void *Buffer, size BufferSize, uint32 Flags);

char id[] = "MDFVER: System-Device/Keyboard-" VERSION;

/*static char *en_unshifted[128] = {
//	         0x0           0x1  0x2  0x3  0x4  0x5  0x6
	(char *)NULL, (char *)NULL, "1", "2", "3", "4", "5",
//	0x7  0x8  0x9  0xa  0xb  0xc  0xd   0xe
	"6", "7", "8", "9", "0", "-", "=", "\b",
//	 0xf 0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17 0x18 0x19 0x1a 0x1b  0x1c
	"\t", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "\n",
//	        0x1d 0x1e 0x1f 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27 0x28 0x29
	(char *)NULL, "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "`",
//	        0x2a  0x2b 0x2c 0x2d 0x2e 0x2f 0x30 0x31 0x32 0x33 0x34 0x35          0x36
	(char *)NULL, "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".", "/", (char *)NULL,
//	        0x37          0x38 0x39          0x3a
	(char *)NULL, (char *)NULL, " ", (char *)NULL, // Дальше пока неизвестно ничего.
};

static char *en_shifted[128] = {
//	         0x0           0x1  0x2  0x3  0x4  0x5  0x6
	(char *)NULL, (char *)NULL, "!", "@", "#", "$", "%",
//	0x7  0x8  0x9  0xa  0xb  0xc  0xd   0xe
	"^", "&", "*", "(", ")", "_", "+", "\b",
//	 0xf 0x10 0x11 0x12 0x13 0x14 0x15 0x16 0x17 0x18 0x19 0x1a 0x1b  0x1c
	"\t", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "\n",
//	        0x1d 0x1e 0x1f 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27 0x28 0x29
	(char *)NULL, "A", "S", "D", "F", "G", "H", "J", "K", "L", ":", "\"", "~",
//	        0x2a  0x2b 0x2c 0x2d 0x2e 0x2f 0x30 0x31 0x32 0x33 0x34 0x35          0x36
	(char *)NULL, "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">", "?", (char *)NULL,
//	        0x37          0x38 0x39          0x3a
	(char *)NULL, (char *)NULL, " ", (char *)NULL, // Дальше пока неизвестно ничего.
};

#define KEY_BUFFER_LEN	32
static char *KeyBuffer[KEY_BUFFER_LEN];
int KeyBufferHead, KeyBufferTail;

lock_t KeyBufferLock;
handle KeyboardIRQ;

bool caps = false;
bool shift = false;

bool extended = false;
*/
void puts (char *str)
{
	char buf[256];
	union namer_message *msg = (union namer_message *)buf;

	msg->Request.Offset = offsetof (union namer_message, Request.Request);
	sprintf (msg->Request.Request, "Console://%s", str);
	msg->Request.Size = strlen (msg->Request.Request) + 1;

	NamerCall (msg, msg->Request.Offset + msg->Request.Size, 0);

	return;
}

static result GetPortAccess (int first_port, int last_port)
{
	result rv;
	handle ph;

	struct ResourceCreatePortParam p = {
		first_port,
		last_port,
		ACCESS_WRITE
	};

	if ((rv = KernelResourceCreate (RESOURCE_PORT, &p,
		sizeof (struct ResourceCreatePortParam), &ph)) != KERNEL_OK)
		return rv;

	return KERNEL_OK;
}

bool RegisterService (void)
{
	handle id;

	if (KernelTPCCreate (0, KeyboardService, 0, &id) != KERNEL_OK)
		return false;

	// Надо разрешить намеру вызывать TPC
	handle namer_proc = NamerProcess();
	if (KernelResourceAttach (id, namer_proc, KERNEL_ACCESS_CALL, 0) != KERNEL_OK)
		return false;

	char req[256];
	union namer_message *msg = (union namer_message *)req;

	sprintf (msg->Request.Request,
		NSSN "Register?prefix='Keyboard://'&tpc=%u",
		(unsigned int)id);
	msg->Request.Offset = offsetof (union namer_message, Request.Request);
	msg->Request.Size = strlen (msg->Request.Request) + 1;

	if (NamerCall (msg, msg->Request.Offset + msg->Request.Size, 0)
		!= KERNEL_OK || msg->Reply.Status != NAMER_OK)
	{
		return false;
	}

	return true;
}


// enum KBD_STATUS {	// Читает содержимое порта 0x64;
// 	PARITY_ERR = 0x80,
// 	TIMEOUT = 0x40,
// 	AUXDATA = 0x20,
// 	KEYLOCK = 0x10,
// 	CMDDAT = 0x08,
// 	MUXERR = 0x04,
// 	IBF = 0x02,	// входной буфер полон
// 	OBF = 0x01,	// выходной буфер полон
// };
//
// enum KBD_COMMAND {	// Команда посылается в порт 0x64 (все результаты возвращаются через порт 0x60)
// 	KBDC_READ_CMD	= 0x20,	// Чтение регистра команд контроллера
// 	KBDC_WRITE_CMD	= 0x60,	// Запись регистра команд контролера (данные писать в порт 0x60)
//
// 	KBDC_TEST_SELF	= 0xaa,	// Тестирование Возвратит 0x55 в случае успеха.
// 	KBDC_KBD_TEST	= 0xab,	// Тестирование интерфейса клавиатуры. в случае успеха - 0.
// 	KBDC_MOUSE_TEST	= 0xa9,	// Тестирование интерфейса мыши. в случае успеха - 0.
//
// 	KBDC_KBD_DISABLE = 0xad, // Запрет клавиатуры.
// 	KBDC_KBD_ENABLE	= 0xae,	// Разрешение клавиатуры
//
// 	KBDC_READ_INPUT	= 0xc0,
// 	KBDC_COPY_0_3	= 0xc1,
// 	KBDC_COPY_4_7	= 0xc2,
//
// 	KBDC_COPY_OUT	= 0xd0,
// 	KBDC_WRITE_OUT	= 0xd1,
//
// 	// только PS/2
// 	KBDC_MOUSE_DISABLE = 0xa7, // Запретить мышь.
// 	KBDC_MOUSE_ENABLE = 0xa8, // Разрешить мышь.
//
// 	KBDC_PWD_TEST	= 0xa4,
// 	KBDC_PWD_TRANSMIT = 0xa5,
// 	KBDC_PWD_MATCH	= 0xa6,
//
// 	KBDC_TEST_DIAG	= 0xac,
//
// 	KBDC_WRITE_KBD_BUFFER = 0xd2,
// 	KBDC_WRITE_MOUSE_BUFFER = 0xd3,
//
// 	KBDC_WRITE_MOUSE_BYTE = 0xd4,
//
// 	KBDC_READ_TEST_INPUT = 0xe0,
//
// 	KBDC_OUTPUT_PULSE = 0xf0,
// };
//
// enum KDBC_COMMAND {	// Эта команда посытаетя в контроллер после KBDC_WRITE_CMD в порт 0x60.
// 	KBDCC_LED	= 0xed,	// состояние лампочек клавиатуры,
// 				// бит 0 - ScrollLock LED
// 				// бит 1 - CapsLock LED
// 				// бит 2 - NumLock LED.
//
// 	KBDCC_ECHO	= 0xee,	// Эхо... возвращает 0xee
//
// 	KBDCC_SETRR	= 0xf3, // Установить частоту повторов.
// 				// бит 1-4: (30cps, ..., 2cps)
// 				// бит 5-6: (0.25s, 0.5s, 0.75s, 1s)
//
// 	KBDCC_ENABLE	= 0xf4,	// Разрешить клавиатуру
// 	KBDCC_RESET_W	= 0xf5, // Сброи и ожидание KBDCC_ENABLE
// 	KBDCC_RESET_S	= 0xf6,	// Сброс и включение
//
// 	KBDCC_RESEND	= 0xfe,	// ПОвторная отправка последнего кода
// 	KBDCC_RESET_TEST= 0xff,	// Сброс и самодиогностика... (когда заканчивается - непонятно)
//
// 	// только PS/2
// 	KBDCC_SCSET	= 0xf0,	// Выбор таблицы сканкодов.
// 	KBDCC_ID	= 0xf2, // Получить код клавиатуры. два байта на чтение.
//
// 	// Здесь еще команды 0xf7-0xfd... ну они мне пока не нужны.
// };
//
// #define ACK	0xfa
//
// #define KEYBOARD_TUMEOUT	10000
//
// bool KeyboardWaitForOutput (void)
// {
// 	for (int i = 0; i < KEYBOARD_TUMEOUT; i++) {
// 		if ((InPortByte (0x64) & OBF) != 0)
// 			return true;
//
// 		KernelSheduleNext (50);
// 	}
//
// 	return false;
// }
//
// bool KeyboardWaitForInput (void)
// {
// 	for (int i = 0; i < KEYBOARD_TUMEOUT; i++) {
// 		if ((InPortByte (0x64) & IBF) == 0)
// 			return true;
//
// 		KernelSheduleNext (50);
// 	}
//
// 	return false;
// }
//
// bool KeyboardReadyForOutput (void)
// {
// 	if ((InPortByte (0x64) & OBF) != 0)
// 		return true;
//
// 	return false;
// }
//
// bool KeyboardWriteCommand (unsigned char cmd)
// {
// 	if (KeyboardWaitForInput ()) {
// 		OutPortByte (0x64, cmd);
// 		return true;
// 	}
//
// 	return false;
// }
//
// bool KeyboardWriteData (unsigned char value)
// {
// 	if (KeyboardWaitForInput ()) {
// 		OutPortByte (0x60, value);
// 		return true;
// 	}
//
// 	return false;
// }
//
// bool KeyboardControllerCommand (unsigned char cmd)
// {
// 	if (KeyboardWriteCommand (0x60))
// 		if (KeyboardWriteData (cmd))
// 			return true;
//
// 	return false;
// }
//
// int KeyboardReadData ()
// {
// 	if (KeyboardReadyForOutput())
// 		return InPortByte (0x60);
//
// 	return -1;
// }
//
void KeyboardEnable ()
{
	int bv = InPortByte (0x61);
	OutPortByte (0x61, bv | 0x80);
	OutPortByte (0x61, bv);
}

bool i8042_init (unsigned short base);
void i8042_retriver (void);

void KeyboardInit (void)
{
// 	KeyBufferHead = KeyBufferTail = 0;
// 	for (int i = 0; i < KEY_BUFFER_LEN; i++)
// 		KeyBuffer[i] = (char*)NULL;
//
// 	Unlock (&KeyBufferLock);

	// Резервирование прерывания
// 	struct ResourceCreateInterruptParam IP;
//
// 	IP.IRQ = 1;
// 	IP.Flags = 0; // Вообще-то флаг нам пофиг.
//
// 	if (KernelResourceCreate (RESOURCE_INTERRUPT, &IP, sizeof (IP), &KeyboardIRQ) == KERNEL_OK) {
// 		puts ("Keyboard: IRQ1 Creation successfull.\n");
// 	} else {
// 		KeyboardIRQ = BAD_HANDLE;
// 	}

	// Резервирование портов ввода-вывода
	if (GetPortAccess (0x60, 0x67) != KERNEL_OK) {
		return;
	}

	i8042_init (0x60);

	// Инициализация
//	KeyboardControllerCommand (0x47);

// 	KeyboardWriteCommand(0x60);
// 	KeyboardWriteData(0x41);

	KeyboardEnable();
/*
	shift = caps = extended = false;*/

	puts ("Keyboard: Init... Ok.\n");
}

// void KeyboardGetSymbol (char *ch)
// {
// 	Lock (&KeyBufferLock);
//
// 	int NKBT = (KeyBufferTail + 1) % KEY_BUFFER_LEN;
//
// 	if (NKBT != KeyBufferHead) {
// 		KeyBuffer[KeyBufferTail] = ch;
// 		KeyBufferTail = NKBT;
// 	} else {
// 		puts ("Keyboard: buffer full.\n");
// 	}
//
// 	Unlock (&KeyBufferLock);
// }
//
// void KeyboardParseCode (int code)
// {
// 	if (code == 0xe0) {
// 		extended = true;
// 		return;
// 	}
//
// 	bool pressed = (code & 0x80) ? false : true;
// 	code &= 0x7f;
//
// 	if (extended) {
// 		code |= 0x80;
// 		extended = false;
// 	}
//
// 	if (pressed) {
// 		char *ch = NULL;
//
// 		switch (code) {
// 			case 0x3a:	// Caps pressed
// 				caps = !caps;
// 				break;
// 			case 0x2a:
// 			case 0x36:
// 				shift = true;
// 				break;
// 			default:
// 				if (code < 128) {
// 					ch = ((caps ^ shift) ?
// 						en_shifted :
// 						en_unshifted)[code];
//
// 					if (ch)
// 						KeyboardGetSymbol (ch);
// 				}
//
// 				if (ch == NULL) {
// 					char buf[80];
// 					sprintf (buf, "\nKeyboard: "
// 						"Unhandled pressed key: %u\n",
// 						code);
// 					puts (buf);
// 				}
//
// 				break;
// 		}
// 	} else {
// 		switch (code) {
// 			case 0x2a:
// 			case 0x36:
// 				shift = false;
// 				break;
// 		}
// 	}
//
// 	return;
// }
//
// void KeyboardInput (void)
// {
// 	if (KeyboardIRQ != BAD_HANDLE) {
// 		while (true) {
// 			// Ждем через прерываниe.
// 			KernelResourceWait (KeyboardIRQ, INTERRUPT_EVENT_IRQ);
//
// 			puts(".");
//
// 			int code;
// 			while ((code = KeyboardReadData()) != -1)
// 				KeyboardParseCode (code);
//
// 			KeyboardEnable();
// 		}
// 	} else {
// 		puts ("Keyboard: PIO mode (without interrupt)\n");
//
// 		while (true) {
// 			KeyboardWaitForOutput ();
//
// 			int c = KeyboardReadData();
//
// 			if (c != -1)
// 				KeyboardParseCode (c);
// 		}
// 	}
// }

int main (int argc, char **argv)
{
	RegisterService ();

	KeyboardInit ();

//	KeyboardInput ();
	i8042_retriver ();

	return 0;
}

void KeyboardService (handle tid, void *Buffer, size BufferSize, uint32 Flags)
{
	// По началу всякая паранойя...
	if (Flags & KERNEL_RESOURCE_CALL_READONLY)
		return;

	if (BufferSize < sizeof (union namer_message))
		return;

	// TODO: Формат надо немного поменять ну чуть погодя. захардкодим пока.
	union namer_message *msg = (union namer_message *)Buffer;

// 	if (msg->Request.Offset > BufferSize ||
// 		msg->Request.Offset + msg->Request.Size > BufferSize)
// 	{
// 		return;
// 	}
//
// 	if (memchr ((char *)msg + msg->Request.Offset, 0, msg->Request.Size)
// 		== NULL)
// 	{
// 		return;
// 	}
//
// 	if (strcmp ((char *)msg + msg->Request.Offset, "get") == 0) {
// 		// Получение кода символа.
// 		while (1) {	// Ждем однако!
// 			Lock (&KeyBufferLock);
//
// 			if (KeyBufferHead != KeyBufferTail &&
// 			    KeyBuffer[KeyBufferHead] != NULL &&
// 			    BufferSize > (strlen (KeyBuffer[KeyBufferHead]) +
// 				offsetof (union namer_message, Reply.Reply)))
// 			{
// 				int NKBH = (KeyBufferHead + 1) %
// 					KEY_BUFFER_LEN;
//
// 				strcpy (msg->Reply.Reply,
// 					KeyBuffer[KeyBufferHead]);
// 				KeyBuffer[KeyBufferHead] =
// 					(char *)NULL; // Паранойя
// 				KeyBufferHead = NKBH;
//
// 				Unlock (&KeyBufferLock);
// 				break;
// 			}
//
// 			Unlock (&KeyBufferLock);
//
// 			KernelSheduleNext (0);
// 		}
//
// 		// NOTE: Когда будет полноценный форвард - это будет иметь важный смысл.
// 		msg->Reply.Status = NAMER_OK;
// 	} else {
// 		// NOTE: А особенно это!  ибо то - намер может установить и предварительно.
		msg->Reply.Status = NAMER_INVALID_PARAM;
		msg->Reply.Reply[0] = 0;	// Нет ответа!
// 	}

	return;
};
