
#include <MDF/Kernel.h>
#include <MDF/IOPorts.h>

#include <stdbool.h>

void puts (char *str);

#define log(s)	puts(s)

#define I8042_DATA_PORT		0
#define I8042_STATUS_PORT	4
#define I8042_CMD_PORT		4

enum {
	I8042_STATUS_OBF = 0x01,
	I8042_STATUS_IBF = 0x02,
	I8042_STATUS_MUXERR = 0x04,
	I8042_STATUS_CMDDAT = 0x08,
	I8042_STATUS_KEYLOCK = 0x10,
	I8042_STATUS_AUXDATA = 0x20,
	I8042_STATUS_TIMEOUT = 0x40,
	I8042_STATUS_PARITY_ERR = 0x80,
};

struct i8042_device {
	int port_number;
	void (*retriver)(struct i8042_device * dev, unsigned char code);
};

void detect_dev (struct i8042_device * dev, unsigned char code);
void keyboard_retriver (struct i8042_device * dev, unsigned char code);
void mouse_retriver (struct i8042_device * dev, unsigned char code);

struct i8042_device detect_dev0 = { 0, detect_dev };
struct i8042_device detect_dev1 = { 1, detect_dev };

struct i8042_device keyboard_dev = { 0, keyboard_retriver };
struct i8042_device mouse_dev = { 0, mouse_retriver };

struct i8042_device *i8042_ports[2] =
{
	&detect_dev0,
	&detect_dev1
};

void keyboard_retriver (struct i8042_device * dev, unsigned char code)
{
	log ("i8042: keyboard retriver under constracted.\n");
}

void mouse_retriver (struct i8042_device * dev, unsigned char code)
{
	log ("i8042: mouse retriver under constracted.\n");
}

void detect_dev (struct i8042_device * dev, unsigned char code)
{
	switch (code) {
		case 0xee:
			i8042_ports[dev->port_number] = &keyboard_dev;
			log ("i8042: keyboard found.\n");
			break;
		case 0xfa:
			i8042_ports[dev->port_number] = &mouse_dev;
			log ("i8042: mouse found.\n");
			break;
		default:
			log ("i8042: unknown sequence\n"); // code
			break;
	}
}


// Низкоуровневые утилиты 8042

bool i8042_wait (int base)
{
	for (int i = 0; i < 250; i++) {
		int status = InPortByte (base + I8042_STATUS_PORT);

		if (status & (I8042_STATUS_PARITY_ERR | I8042_STATUS_TIMEOUT))
			return false;

		if (status & I8042_STATUS_OBF)
			return true;

		KernelSheduleNext (1);
	}

	return false;
}

bool i8042_wait_in (int base)
{
	for (int i = 0; i < 250; i++) {
		int status = InPortByte (base + I8042_STATUS_PORT);

		if (status & I8042_STATUS_IBF) {
		} else {
			return true;
		}

		KernelSheduleNext (1);
	}

	return false;
}

bool i8042_read (int base, int *value)
{
	*value = -1;
	if (i8042_wait (base) == false)
		return false;

	*value = InPortByte (base + I8042_DATA_PORT);
	return true;
}


// Интерфейс драйвера
bool i8042_write (int base, int port_n, unsigned char data)
{
//	int value;

	switch (port_n) {
		case 1:
			OutPortByte (base + I8042_CMD_PORT, 0xd4);
		case 0:
			OutPortByte (base + I8042_DATA_PORT, data);
			break;
		default:
			log ("i8042: Invalid i8042 port.\n");
			return false;
	}

	return true;
}

bool i8042_deliverer (void)
{
	for (int i = 0; i < 250; i++) {
		int status = InPortByte (0x60 + I8042_STATUS_PORT);

		if (status & (I8042_STATUS_PARITY_ERR | I8042_STATUS_TIMEOUT))
			return false;

		if (status & I8042_STATUS_OBF) {
			int code = InPortByte (0x60 + I8042_DATA_PORT);
			int port = ((status & I8042_STATUS_AUXDATA) == 0) ? 0 : 1;
			i8042_ports[port]->retriver (i8042_ports[port], code);
			return true;
		}

		KernelSheduleNext (1);
	}

	return false;
}

void i8042_detect (int base, int port)
{
	i8042_write (base, port, 0xee);
	i8042_deliverer ();
}

bool i8042_init (int base)
{
	int value;

	// Очистка буфера... если там что-то есть
	// Правда не совсем понимаю насколько это красиво.
	// я ж еще не уверен что контроллер присутствует.
	while (true) {
		int status = InPortByte (base + I8042_STATUS_PORT);

		if (status & I8042_STATUS_OBF)
			InPortByte (base + I8042_DATA_PORT);
		else
			break;
	}

	// Попытаемся продетектить контроллер.
	OutPortByte (base + I8042_CMD_PORT, 0xaa);
	if (i8042_read (base, &value) != false && value == 0x55)
	{
		log ("i8042: Detected.\n");
	} else {
		log ("i8042: Not detected.\n");
		return false;
	}

	// Пропишем регистр команды.
	OutPortByte (base + I8042_CMD_PORT, 0x60);
	i8042_wait_in (base);
	// прерывания разрешены, порты запрещены.
	OutPortByte (base + I8042_DATA_PORT, 0x37);

	// Попытаемся продетектить KBD порт
	OutPortByte (base + I8042_CMD_PORT, 0xab);
	if (i8042_read (base, &value) != false && value == 0x00)
	{
		log ("i8042: port 0 test success.\n");

		i8042_detect (base, 0);
		OutPortByte (base + I8042_CMD_PORT, 0xae);
	} else {
		log ("i8042: port 0 test failed.\n");
	}

	// Попытаемся продетектить AUX порт
	OutPortByte (base + I8042_CMD_PORT, 0xa9);
	if (i8042_read (base, &value) != false && value == 0x00)
	{
		log ("i8042: port 1 test success.\n");

		i8042_detect (base, 1);
		OutPortByte (base + I8042_CMD_PORT, 0xa8);
	} else {
		log ("i8042: port 1 test failed.\n");
	}

	return true;
}

void i8042_retriver (void)
{
	HANDLE KeyboardIRQ;
	struct ResourceCreateInterruptParam IP;

	IP.IRQ = 1;
	IP.Flags = 0; // Вообще-то флаг нам пофиг.

	if (KernelResourceCreate (RESOURCE_INTERRUPT, &IP, sizeof (IP), &KeyboardIRQ) == KERNEL_OK)
	{
		log ("i8042: IRQ1 allocated.\n");
	} else {
		log ("i8042: IRQ1 not allocated.\n");
		KeyboardIRQ = BAD_HANDLE;
	}

	if (KeyboardIRQ != BAD_HANDLE) {
		log ("i8042: Interrupt mode...\n");
		while (true) {
			// Ждем через прерываниe.
			KernelResourceWait (KeyboardIRQ, INTERRUPT_EVENT_IRQ, 0xffffffff);

//			log (".");

			if (i8042_deliverer () == false) {
				log ("i8042: missing interrupt..\n");
			}

			// Разрешить прерывание
			int bv = InPortByte (0x61);
			OutPortByte (0x61, bv | 0x80);
			OutPortByte (0x61, bv);
		}
	} else {
		log ("i8042: PIO mode (without interrupt) to the hell...\n");
		log ("i8042: General fault..\n");
	}
}
