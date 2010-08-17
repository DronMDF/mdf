//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include "StubLocal.h"
#include "Time.h"
#include "GDT.h"
#include "Descriptor.h"

void StubSetIDT (volatile void *idt, size_t idt_size);

enum IDT_IDX {
	IDT_EXCEPTION_BASE	= 0,
	IDT_INTERUPT_BASE	= 32,
	IDT_SYSCALL_BASE	= 48,

	IDT_SIZE		= 64,
};

static descriptor_t IDT[IDT_SIZE] __attribute__((aligned(16)));

// Включаемые функции экстерним здесь - так проще.
extern void StubExceptionDE;
extern void StubExceptionDB;
extern void StubExceptionNMI;
extern void StubExceptionBP;
extern void StubExceptionOF;
extern void StubExceptionBR;
extern void StubExceptionUD;
extern void StubExceptionNM;
extern void StubExceptionDF;
extern void StubExceptionCSO;
extern void StubExceptionTS;
extern void StubExceptionNP;
extern void StubExceptionSS;
extern void StubExceptionGP;
extern void StubExceptionPF;
extern void StubExceptionMF;
extern void StubExceptionAC;
extern void StubExceptionMC;
extern void StubExceptionXF;

extern void StubInterrupt0;
extern void StubInterrupt1;
extern void StubInterrupt2;
extern void StubInterrupt3;
extern void StubInterrupt4;
extern void StubInterrupt5;
extern void StubInterrupt6;
extern void StubInterrupt7;
extern void StubInterrupt8;
extern void StubInterrupt9;
extern void StubInterrupt10;
extern void StubInterrupt11;
extern void StubInterrupt12;
extern void StubInterrupt13;
extern void StubInterrupt14;
extern void StubInterrupt15;

extern void KernelWait;
extern void KernelFind;
extern void KernelCreate;
extern void KernelCall;
extern void KernelAttach;
extern void KernelDetach;
extern void KernelModify;
extern void KernelInfo;

// TODO - в Descriptor.c
static
void __init__ StubSetGateDescriptor(descriptor_t *dt, int vector,
	int eselector, laddr_t eoffset, int flags)
{
	STUB_ASSERT (isSet(flags, 0xf00), "Hi flags in gate");
	flags |= DESCRIPTOR_PRESENT;

	const int idx = vector;

	dt[idx].raw = 0;

	dt[idx].gate.selector	=  (uint16_t)eselector;

	dt[idx].gate.offsetlo	=  eoffset & 0x0000ffff;
	dt[idx].gate.offsethi	= (uint16_t)((eoffset & 0xffff0000) >> 16);

	dt[idx].gate.flags	=  (uint8_t)(flags & 0xff);
}

// TODO - в Descriptor.c
static
void __init__ StubSetGateException(const int no, const laddr_t addr)
{
	StubSetGateDescriptor (IDT, no, KERNEL_CODE_SELECTOR, addr,
		DESCRIPTOR_INTERRUPT | DESCRIPTOR_PL0);
}

// TODO - в Descriptor.c
static
void __init__ StubSetGateInterrupt(const int no, const laddr_t addr)
{
	StubSetGateDescriptor (IDT, IDT_INTERUPT_BASE + no,
		KERNEL_CODE_SELECTOR, addr, DESCRIPTOR_INTERRUPT | DESCRIPTOR_PL0);
}

// TODO - в Descriptor.c
static
void __init__ StubSetGateSyscall(const int no, const laddr_t addr)
{
	StubSetGateDescriptor (IDT, IDT_SYSCALL_BASE + no,
		KERNEL_CODE_SELECTOR, addr, DESCRIPTOR_INTERRUPT | DESCRIPTOR_PL3);
}

void __init__ StubInitIDT()
{
	// С IDT проще
	// 32 исключения
	// 16 системных вызовов...
	// 16 прерываний (хотя если вспомнить про APIC?)
	// пока сделаю 64.
	// если возникнет прерывание больше -
	// то на процессоре случиться #GP и все счастливы.

	// Отлавливание нарушений следующее:
	// Если vector * 8 + 7 находится за пределами лимита IDT или
	// Дескриптор IDT не интеррапт/трап/таскгейт
	// То получаем #GP(vector * 8 + 2 + EXT)
	// Если права доступа не допускают вызова - #GP(vector * 8 + 2)
	// Там еще куча вариантов исключений, но все они относятся к валидным векторам

	// Инициализируем таблицу IDT
	StubSetGateException (0, v2laddr(&StubExceptionDE));
	StubSetGateException (1, v2laddr(&StubExceptionDB));
	StubSetGateException (2, v2laddr(&StubExceptionNMI));
	StubSetGateException (3, v2laddr(&StubExceptionBP));
	StubSetGateException (4, v2laddr(&StubExceptionOF));
	StubSetGateException (5, v2laddr(&StubExceptionBR));
	StubSetGateException (6, v2laddr(&StubExceptionUD));
	StubSetGateException (7, v2laddr(&StubExceptionNM));
	StubSetGateException (8, v2laddr(&StubExceptionDF));
	StubSetGateException (9, v2laddr(&StubExceptionCSO));
	StubSetGateException (10, v2laddr(&StubExceptionTS));
	StubSetGateException (11, v2laddr(&StubExceptionNP));
	StubSetGateException (12, v2laddr(&StubExceptionSS));
	StubSetGateException (13, v2laddr(&StubExceptionGP));
	StubSetGateException (14, v2laddr(&StubExceptionPF));
	StubSetGateException (16, v2laddr(&StubExceptionMF));
	StubSetGateException (17, v2laddr(&StubExceptionAC));
	StubSetGateException (18, v2laddr(&StubExceptionMC));
	StubSetGateException (19, v2laddr(&StubExceptionXF));

	// Все прерывания...
	StubSetGateInterrupt (0, v2laddr(&StubInterrupt0));
	StubSetGateInterrupt (1, v2laddr(&StubInterrupt1));
	StubSetGateInterrupt (2, v2laddr(&StubInterrupt2));
	StubSetGateInterrupt (3, v2laddr(&StubInterrupt3));
	StubSetGateInterrupt (4, v2laddr(&StubInterrupt4));
	StubSetGateInterrupt (5, v2laddr(&StubInterrupt5));
	StubSetGateInterrupt (6, v2laddr(&StubInterrupt6));
	StubSetGateInterrupt (7, v2laddr(&StubInterrupt7));
	StubSetGateInterrupt (8, v2laddr(&StubInterrupt8));
	StubSetGateInterrupt (9, v2laddr(&StubInterrupt9));
	StubSetGateInterrupt (10, v2laddr(&StubInterrupt10));
	StubSetGateInterrupt (11, v2laddr(&StubInterrupt11));
	StubSetGateInterrupt (12, v2laddr(&StubInterrupt12));
	StubSetGateInterrupt (13, v2laddr(&StubInterrupt13));
	StubSetGateInterrupt (14, v2laddr(&StubInterrupt14));
	StubSetGateInterrupt (15, v2laddr(&StubInterrupt15));

	// И все системные вызовы.
	StubSetGateSyscall (0, v2laddr(&KernelWait));
	StubSetGateSyscall (1, v2laddr(&KernelFind));
	StubSetGateSyscall (2, v2laddr(&KernelCreate));
	StubSetGateSyscall (3, v2laddr(&KernelCall));
	StubSetGateSyscall (4, v2laddr(&KernelAttach));
	StubSetGateSyscall (5, v2laddr(&KernelDetach));
	StubSetGateSyscall (6, v2laddr(&KernelModify));
	StubSetGateSyscall (7, v2laddr(&KernelInfo));

	StubSetIDT (&IDT, IDT_SIZE * sizeof(descriptor_t));
	CorePrint ("IDT initialized.\n");
}

void StubUnhandledException(const int num, const unsigned long error)
{
	// TODO: Переделать на условия.
	STUB_ASSERT (num < 0 || num >= 0x20, "Illegal exception number");

	static const char *exception[] = {
		[0] = "Division Error",
		[1] = "Debug",
		[2] = "NMI Interrupt",
		[3] = "Breakpoint",
		[4] = "Overflow",
		[5] = "BOUND Range Exceeded",
		[6] = "Invalid/Undefined Opcode",
		[7] = "Device Not Available (No Math Coprocessor)",
		[8] = "Double Fault",
		[9] = "Coprocessor Segment Overrun",
		[10] = "Invalid TSS",
		[11] = "Segment Not Present",
		[12] = "Stack-Segment Fault",
		[13] = "General Protection",
		[14] = "Page Fault",
		[15] = "Intel Reserved",
		[16] = "x87 FPU Floating Point Error (Math Fault)",
		[17] = "Alignment Check",
		[18] = "Machine Check",
		[19] = "SIMD Floating Point"
	};

	CorePrint ("Unhandled %s Exception (#%u) (error: 0x%8x)\n",
		exception[num] != 0 ? exception[num] : "Unknown", num, error);

	// Игнорируем Debug exception, бош его вызывает.
	if (num == 1)
		return;

	StubSoD (exception[num], __FILE__, __LINE__);
}

void StubInterruptHandler(const int irq)
{
	StubInterruptAcknowledge (irq);

	if (StubInterruptIsMasked(irq)) {
		// Спонтанные irq поддерживаются в qemu...
		CorePrint("Spurious IRQ #%u\n", irq);
		return;
	}

	if (irq == 0) {
		StubClockHandler();
		return;
	}

	CorePrint ("Unhandled Interrupt #%u\n", irq);
	STUB_FATAL ("Unhandled Interrupt");
}
