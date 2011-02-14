//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include <Kernel.h>

#include "StubLocal.h"
#include "Arch.h"
#include "Utils.h"
#include "Page.h"

extern void __init_ro;
extern void __text_end;

// отладка

static const bool stubTraceEnable = false;

// Чтобы не возиться длкально с va_args воспользуемся дефайном.
#define StubCallStackTrace(format, ...)				\
	if (stubTraceEnable) {					\
		CorePrint ("TRACE: " format, __VA_ARGS__);	\
	} else {}

// Поддержка символов для бсода.
typedef struct _SymbolInfo SymbolInfo;

struct _SymbolInfo {
	laddr_t addr;
	const char *name;
} __attribute__((packed));

static
unsigned int SymCount = 0;

static
unsigned int SymCountMax __initdata__;

static
SymbolInfo *SymTab = nullptr;

void __init__ StubSoD_SymbolCount(unsigned int symcount)
{
	SymTab = (SymbolInfo *)StubMemoryAlloc (sizeof (SymbolInfo) * symcount);
	SymCountMax = symcount;
}

void __init__ StubSoD_AddSymbol (unsigned long addr, const char *name)
{
	STUB_ASSERT (SymTab == nullptr, "Not set symbol count (StubSoD_SymbolCount)");
	STUB_ASSERT (SymCount >= SymCountMax, "Many symbols");

	SymTab[SymCount].addr = addr;
	SymTab[SymCount].name = name;
	SymCount++;
}

// Вывод
static
const char *StubSoD_Symbol (const laddr_t addr)
{
	if (SymTab != nullptr) {
		for (unsigned int i = 0; i < SymCount; i++) {
			if (SymTab[i].addr == addr) {
				return SymTab[i].name;
			}
		}
	}

	return 0;
}

static
laddr_t StubSoD_NearSymbolAddr (const laddr_t addr)
{
	laddr_t m = 0;
	for (unsigned int i = 0; i < SymCount; i++) {
		if (m < SymTab[i].addr && SymTab[i].addr <= addr)
			m = SymTab[i].addr;
	}

	return m;
}

static
void StubSoDPrintDemangledName (laddr_t entry)
{
	const char *symbol = StubSoD_Symbol(entry);

	if (symbol == 0) {
		// Нет символов
		CorePrint ("0x%08x", entry);
		return;
	}

	if (StubStringEqual (symbol, "_ZNK", 4)) {
		symbol += 4;
	} else if (StubStringEqual (symbol, "_ZN", 3)) {
		symbol += 3;
	} else {
		CorePrint ("%s", symbol);
		return;
	}

	int sl = 0;
	bool isFirst = true;
	while (*symbol != '\0'){
		if ('0' <= *symbol && *symbol <= '9') {
			sl = sl * 10 + (*symbol - '0');
			symbol++;
			continue;
		}

		if (sl == 0)
			break;

		// CorePrint ("\nSymbol len %u\n", sl);

		if (!isFirst)
			CorePrint ("::");

		isFirst = false;

		for (; sl > 0 && *symbol != '\0'; sl--) {
			StubPrintChar (*symbol);
			symbol++;
		}
	}
}

//------------------------------------------------------------------------------
// Разворот стека для StubSoD

// Эта Вспомогательная таблица для определения длины инструкций.

#define MR1	0x11
#define MR2	0x12

#define I11	0x21	// modrm с последующим imm8
#define I12	0x22	// modrm с последующим imm8
#define I41	0x41	// modrm с последующим imm32

#define FPU	   2	// Там попадаются варианты с mod/rm, но для нас это не важно..

static const unsigned char iLength[512] = {
//	x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0a 0x0b 0x0c 0x0d 0x0e 0x0f
	MR1, MR1, MR1, MR1,   2,   5,   1,   1, MR1, MR1, MR1, MR1,   2,   5,   1,0xff,	// 0x00
	MR1, MR1, MR1, MR1,   2,   5,   1,   1, MR1, MR1, MR1, MR1,   2,   5,   1,   1,	// 0x10
	MR1, MR1, MR1, MR1,   2,   5,   1,   1, MR1, MR1, MR1, MR1,   2,   5,   0,   2,	// 0x20
	MR1, MR1, MR1, MR1,   2,   5,   1,   1, MR1, MR1, MR1, MR1,   2,   5,   0,   1,	// 0x30
	  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,	// 0x40
	  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,	// 0x50
	  1,   1,   0,   0,   1,   1,   1,   1,   5, I41,   2, I11,   1,   1,   1,   1,	// 0x60
	  2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,	// 0x70
	I11, I41, I11, I11, MR1, MR1, MR1, MR1, MR1, MR1, MR1, MR1, MR1, MR1,   2,   0,	// 0x80
	  1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   1,   1,   1,   1,	// 0x90
	  5,   5,   5,   5,   1,   1,   0,   1,   2,   5,   1,   1,   1,   0,   0,   0,	// 0xa0
	  2,   2,   2,   2,   2,   2,   2,   2,   5,   5,   5,   5,   5,   5,   5,   5,	// 0xb0
	I11, I11,   3,   1, MR1, MR1, I11, I41,   4,   1,   0,   1,   0,   0,   0,   1,	// 0xc0
	MR1, MR1, MR1, MR1,   2,   2,   0,   0, FPU, FPU, FPU, FPU, FPU, FPU, FPU, FPU,	// 0xd0
	  2,   2,   2,   2,   2,   0,   2,   2,   5,   5,   0,   2,   1,   1,   1,   0,	// 0xe0
	  1,   0,   1,   1,   1,   1, I11, I41,   1,   1,   1,   1,   1,   1,   0, MR1,	// 0xf0

// Первый байт 0x0f, а здесь по второму...
//	x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0a 0x0b 0x0c 0x0d 0x0e 0x0f
	  2,   0,   0,   0,   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 0x00
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 0x10
	  3,   0,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 0x20
	  0,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 0x30
	MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2,	// 0x40
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 0x50
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 0x60
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 0x70
	  6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,   6,	// 0x80
	MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2, MR2,	// 0x90
	  2,   0,   0,   0, I12, MR2,   0,   0,   2,   0,   0,   0, I12, MR2,   0, MR2,	// 0xa0
	MR2, MR2,   0,   0,   0,   0, MR2, MR2,   0,   0,   0,   0, MR2, MR2,   3, MR2,	// 0xb0
	  0,   0,   0,   0,   0,   0,   0,   0,   2,   2,   2,   2,   2,   2,   2,   2,	// 0xc0
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 0xd0
	  0,   0,   0,   0,   0,   0,   0,   0, MR2,   0,   0,   0,   0,   0,   0,   0,	// 0xe0
	  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,	// 0xf0
};

// определение длинны инструкции IA32 (только 32-х битный режим)
static
unsigned int StubSoDInstructionLength(const laddr_t caddr)
{
	if (!StubMemoryReadable (caddr, 3))
		return 0;

	const unsigned char *cptr = l2vptr(caddr);
	int code = (cptr[0] != 0x0f) ? cptr[0] : (cptr[1] + 256);
	uint8_t cl = iLength[code];

	if (cl == 0) {
		CorePrint ("Unknown %s-byte instruction code 0x%02x at 0x%08x\n",
			(code < 256) ? "one" : "two", code & 0xff, caddr);

		while (1);
		return 1;
	}

	if ((cl & 0xf0) == 0)
		return cl;

	unsigned int modrm = cl & 0x0f;
	STUB_ASSERT (modrm == 0, "Illegal mod r/m");

	unsigned int imm = 0;
	if ((cl & 0x20) != 0) {
		imm = 1;
	}
	if ((cl & 0x40) != 0) {
		imm = 4;
	}

	const int rm = cptr[modrm] & 0x07;
	switch (cptr[modrm] & 0xc0) {
		case 0x00:
			if (rm == 0x04)
				return modrm + imm + 2;
			if (rm == 0x05)
				return modrm + imm + 5;
			break;
		case 0x40:
			return modrm + imm + (rm == 0x04 ? 3 : 2);
		case 0x80:
			return modrm + imm + (rm == 0x04 ? 6 : 5);
		case 0xc0:
			break;
	}

	return modrm + imm + 1;
}

static
bool isIns (const unsigned char * const ins, const laddr_t addr)
{
	STUB_ASSERT (ins[0] != 1 && ins[0] != 2, "Illegal instruction pattern");

	// Использование чекера здесь - не самый быстродейственный путь,
	// Но зато существенно уменьшает количество кода после.
	if (!StubMemoryReadable (addr, ins[0]))
		return false;

	const unsigned char *cptr = l2vptr(addr);

	// Длина паттернов пока строго 1 или 2, нет смысла извращаться с циклами.
	if ((cptr[0] & ins[1]) != ins[2])
		return false;

	if (ins[0] > 1 && (cptr[1] & ins[3]) != ins[4])
		return false;

	return true;
}

static
signed long StubSoDFullDisplacment (const laddr_t addr)
{
	return *(signed long *)(addr);
}

static
signed long StubSoDShortDisplacment (const laddr_t addr)
{
	return *(signed char *)(addr);
}

static
unsigned long StubSoDImm32 (const laddr_t addr)
{
	return *(unsigned long *)(addr);
}

static
unsigned long StubSoDImm16 (const laddr_t addr)
{
	return *(unsigned short *)(addr);
}

static
unsigned long StubSoDImm8 (const laddr_t addr)
{
	return *(unsigned char *)(addr);
}

// Функции трассировки инструкций...
static
bool StubSoDReturnIns (const laddr_t ip)
{
	const unsigned char iRet[]	= { 1, 0xff, 0xc3 };
	const unsigned char iRetImm[]	= { 1, 0xff, 0xc2 };

	return isIns(iRet, ip) || isIns(iRetImm, ip);
}

static
laddr_t StubSoDCallIns (const laddr_t ip, const unsigned int il)
{
	const unsigned char iCall[]	= { 1, 0xff, 0xe8 };
	const unsigned char iCallInd[]	= { 2, 0xff, 0xff, 0x38, 0x10 };

	if (isIns (iCall, ip)) {
		return ip + il + StubSoDFullDisplacment (ip + 1);
	}

	if (isIns (iCallInd, ip)) {
		return 1;
	}

	return 0;
}

static
laddr_t StubSoDBranchIns(const laddr_t ip, const unsigned int il)
{
	const unsigned char iJccShort[]		= { 1, 0xf0, 0x70};
	const unsigned char iJcxzShort[]	= { 1, 0xff, 0xe3};
	const unsigned char iJccFull[]		= { 2, 0xff, 0x0f, 0xf0, 0x80 };

	if (isIns(iJccShort, ip) || isIns(iJcxzShort, ip)) {
		return ip + il + StubSoDShortDisplacment (ip + 1);
	}

	if (isIns (iJccFull, ip)) {
		return ip + il + StubSoDFullDisplacment (ip + 2);
	}

	return 0;
}

static
laddr_t StubSoDOtherIns (const laddr_t * * const sptr, const laddr_t ip, const unsigned int il)
{
	// Есть предположение что эта функция сожрет много стека. поглядим.

	const unsigned char iPushRegShort[]	= { 1, 0xf8, 0x50 };
	const unsigned char iPushRegLong[]	= { 2, 0xff, 0xff, 0x38, 0x30 };
	const unsigned char iPushf[]		= { 1, 0xff, 0x9c };
	const unsigned char iPushImm[]		= { 1, 0xfd, 0x68 };
	const unsigned char iPushSegShort[]	= { 1, 0xe7, 0x06 };
	const unsigned char iPushSegLong[]	= { 2, 0xff, 0x0f, 0xc7, 0x80 };
	if (isIns(iPushRegShort, ip) ||
	    isIns(iPushRegLong, ip) ||
	    isIns(iPushf, ip) ||
	    isIns(iPushImm, ip) ||
	    isIns(iPushSegShort, ip) ||
	    isIns(iPushSegLong, ip))
	{
		//StubPrint ("CS: 0x%x push\n", cip);
		*sptr -= 1;
	}

	const unsigned char iPopRegShort[]	= { 1, 0xf8, 0x58 };
	const unsigned char iPopRegLong[]	= { 2, 0xff, 0xff, 0x38, 0x38 };
	const unsigned char iPopf[]		= { 1, 0xff, 0x9d };
	const unsigned char iPopSegShort[]	= { 1, 0xe7, 0x07 };
	const unsigned char iPopSegLong[]	= { 2, 0xff, 0x0f, 0xc7, 0x81 };
	const unsigned char iExt[]		= { 1, 0xff, 0x0f };
	if (isIns(iPopRegShort, ip) ||
	    isIns(iPopRegLong, ip) ||
	    isIns(iPopf, ip) ||
	    (!isIns(iExt, ip) && isIns(iPopSegShort, ip)) || // Багфикс... pop cs == 0x0f
	    isIns(iPopSegLong, ip))
	{
		//StubPrint ("CS: 0x%x pop\n", cip);
		*sptr += 1;
	}

	const unsigned char iPusha[]	= { 1, 0xff, 0x60 };
	if (isIns(iPusha, ip)) {
		*sptr -= 8;
	}

	const unsigned char iPopa[]	= { 1, 0xff, 0x61 };
	if (isIns(iPopa, ip)) {
		*sptr += 8;
	}

	const unsigned char iSubImm8[]	= { 2, 0xff, 0x83, 0xff, 0xec };
	if (isIns(iSubImm8, ip)) {
		*sptr -= StubSoDImm8 (ip + 2) / sizeof (laddr_t);
	}

	const unsigned char iSubImm32[]	= { 2, 0xff, 0x81, 0xff, 0xec };
	if (isIns(iSubImm32, ip)) {
		*sptr -= StubSoDImm32 (ip + 2) / sizeof (laddr_t);
	}

	const unsigned char iAddImm8[]	= { 2, 0xff, 0x83, 0xff, 0xc4 };
	if (isIns(iAddImm8, ip)) {
		*sptr += StubSoDImm8 (ip + 2) / sizeof (laddr_t);
	}

	const unsigned char iAddImm32[]	= { 2, 0xff, 0x81, 0xff, 0xc4 };
	if (isIns(iAddImm32, ip)) {
		*sptr += StubSoDImm32 (ip + 2) / sizeof (laddr_t);
	}

	const unsigned char iEnter[]	= { 1, 0xff, 0xc8 };
	if (isIns(iEnter, ip)) {
		*sptr -= 1 + StubSoDImm16 (ip + 1) / sizeof (laddr_t);
	}

	const unsigned char iLeave[]	= { 1, 0xff, 0xc9 };
	if (isIns(iLeave, ip)) {
		// Мы не в состоянии адекватно обработать команду leave...
		return 0;
	}

	return ip + il;
}

static
laddr_t StubSoDJumpIns (const laddr_t ip, unsigned int il)
{
	const unsigned char iJmpShort[]	= { 1, 0xff, 0xeb };
	if (isIns(iJmpShort, ip)) {
		return ip + il + StubSoDShortDisplacment (ip + 1);
	}

	const unsigned char iJmpFull[]	= { 1, 0xff, 0xe9 };
	if (isIns(iJmpFull, ip)) {
		return ip + il + StubSoDFullDisplacment (ip + 1);
	}

	const unsigned char iJmpRela[] = { 2, 0xff, 0xff, 0x38, 0x20};
	if (isIns(iJmpRela, ip)) {
		// Парсить это нет смысла, просто говорим что куда-то ушли.
		return 1;
	}

	return 0;
}

static
laddr_t StubSoDRetPoint (const laddr_t addr)
{
	if (addr < v2laddr(&__init_ro) || v2laddr(&__text_end) < addr)
		return 0;

	for (unsigned int l = 6; l >= 2; l--) {
		unsigned int il = StubSoDInstructionLength(addr - l);
		if (il != 0) {
			if (StubSoDCallIns (addr - l, il) && il == l) {
				return addr - l;
			}
		}
	}

	return 0;
}

enum {
	MAX_BRANCH_COUNT = 32,
};

typedef struct {
	const laddr_t *esp;
	laddr_t eip;
	laddr_t cs;

	laddr_t bat[MAX_BRANCH_COUNT];
	int bac;
} state_t;

enum {
	SOD_STOP,
	SOD_CALL,
	SOD_JUMP,
	SOD_EXC,
};

static
void StubSoDPrintPoint (int type, laddr_t entry, laddr_t eip)
{
	switch (type) {
		case SOD_STOP: CorePrint ("\tStoped at "); break;
		case SOD_CALL: CorePrint ("\tCalled from "); break;
		case SOD_JUMP: CorePrint ("\tJumped from "); break;
		case SOD_EXC: CorePrint ("\tException at "); break;
	}

	StubSoDPrintDemangledName (entry);
	CorePrint ("()+%u (0x%08x)\n", eip - entry, eip);
}

static
bool StubSoDCallStackRecur (state_t *state, const laddr_t *sptr);

static
bool StubSoDCallStackTrace (state_t *state, const laddr_t *sptr,
	laddr_t addr, laddr_t entry)
{
	if ((v2laddr(&sptr) & (PAGE_SIZE - 1)) < 0x300) {
		CorePrint ("\tStack down at 0x%08x, stop processing...\n", addr);
		return true;
	}

	STUB_ASSERT (addr == 0, "Trace wothout addr impossible!");

	laddr_t eaddr = StubSoD_NearSymbolAddr(addr);
	if (entry > eaddr) {
		eaddr = entry;
	}

	if (eaddr == 0) {
		eaddr = addr;
	}

	// двигаемся по инструкциям..
	laddr_t cip = addr;
	while (cip != state->eip) {
		if (StubSoDReturnIns(cip)) return false;

		const unsigned int il = StubSoDInstructionLength(cip);
		if (il == 0) return false;

		if (sptr - state->esp >= 3 && (sptr[-1] & 0xffff) == state->cs && sptr[-2] == cip)
		{
			// Предположительно исключение
			if (StubSoDCallStackRecur(state, sptr - 3)) {
				StubSoDPrintPoint (SOD_EXC, eaddr, cip);
				return true;
			}
			// Предположение оказалось не верным.
		}

		const laddr_t ca = StubSoDCallIns (cip, il);
		if (ca != 0) {
			const char *sym = StubSoD_Symbol(ca);
			if (sym != 0 && StubStringEqual(sym, "StubSoD", 7)) {
				// Функция StubSoD объявлена как __noreturn__
				// И компилятор это учитывает.
				return false;
			}

			if (sptr - state->esp >= 1 && sptr[0] == cip + il) {
				// Действительный переход.
				if (ca == 1) {
					if (StubSoDCallStackRecur(state, sptr - 1)) {
						StubSoDPrintPoint(SOD_CALL, eaddr, cip);
						return true;
					}
				} else {
					if (StubSoDCallStackTrace(state, sptr - 1, ca, ca)) {
						StubSoDPrintPoint(SOD_CALL, eaddr, cip);
						return true;
					}
				}
			}
		}

		const laddr_t ja = StubSoDJumpIns(cip, il);
		if (ja != 0) {
			if (ja == 1) {
				// Адрес перехода неизвестен - идем по стеку.
				if (StubSoDCallStackRecur(state, sptr - 1)) {
					StubSoDPrintPoint(SOD_JUMP, eaddr, cip);
					return true;
				}
			} else {
				if (ja == cip) return false;

				if (StubSoDCallStackTrace (state, sptr, ja, 0)) {
					StubSoDPrintPoint (SOD_JUMP, eaddr, cip);
					return true;
				}
			}

			return false;
		}

		const laddr_t ba = StubSoDBranchIns(cip, il);
		if (ba != 0) {
			for (int i = 0; i < state->bac; i++) {
				if (cip == state->bat[i]) return false;
			}

			if (state->bac == MAX_BRANCH_COUNT) {
				CorePrint ("\tBranch overrun at 0x%08x, stop processing...\n", cip);
				return true;
			}

			state->bat[state->bac] = cip;
			state->bac += 1;

			const laddr_t bae = (ba < eaddr) ? ba : eaddr;
			if (StubSoDCallStackTrace(state, sptr, ba, bae)) {
				return true;
			}

			state->bac -= 1;
		}

		cip = StubSoDOtherIns (&sptr, cip, il);
	}

	StubSoDPrintPoint (SOD_STOP, eaddr, cip);
	return true;
}

static
bool StubSoDCallStackRecur (state_t *state, const laddr_t *sptr)
{
	for (; sptr >= state->esp; sptr--) {
		laddr_t ra = StubSoDRetPoint (sptr[0]);

		if (ra == state->eip) {
			// Финализация
			return StubSoDCallStackTrace (state, sptr, state->eip, 0);
		}

		if (ra != 0) {
			//CorePrint ("\t\tReturn address 0x%08x\n", ra);
			if (StubSoDCallStackTrace (state, sptr, ra, 0))
				return true;
		}
	}

	return false;
}

static
void StubSoDCallStack (laddr_t *stack, laddr_t eip, laddr_t cs)
{
	state_t state = {
		.esp = stack,
		.eip = eip,
		.cs = cs,

		.bat = { 0 },
		.bac = 0,
	};

	const laddr_t *sptr = l2vptr((v2laddr(stack) & (uint32_t)~0xfff) | 0xffc);
	StubSoDCallStackRecur (&state, sptr);
}

//------------------------------------------------------------------------------
// Собственно экран смерти

void StubSoD_Imm (
	laddr_t edi, laddr_t esi, laddr_t ebp, laddr_t esp,
	laddr_t ebx, laddr_t edx, laddr_t ecx, laddr_t eax,
	laddr_t eflags, laddr_t ss,
	laddr_t fs, laddr_t gs,
	laddr_t es, laddr_t ds,
	laddr_t cs, laddr_t eip,
	const char *msg, const char *file, int	line)
{
	// Коррекция указателя
	esp = (unsigned long)&eip;

	CorePrint ("\n! SoD       Stub-IA32-" VERSION " and %s\n", CoreVersion());
	CorePrint (  "  Message:  '%s' at %s:%u\n", msg, file, line);

	CorePrint ("\n  CallStack:\n");
	//CorePrint ("    ss:esp = 0x%04x:0x%08x\n", ss & 0xffff, esp);
	StubSoDCallStack ((laddr_t *)esp, eip, cs);

	CorePrint ("\n  Mailto: Andrey Valyaev <dron@securitycode.ru>");

	while (true) {
		CorePrint ("\n  ESC - reboot, D - debug, R - registers.");

		int key = StubGetChar();
		if (key != 'R')
			continue;

		CorePrint ("\n\n  Registers:\n");
		// 123456789012345678901234567890
		// ds = 0x????
		// eax = 0x????????
		// eflags = 0x????????
		// cs:eip = 0x????:0x?????????

		CorePrint ("    eax = 0x%08x     esi = 0x%08x ", eax, esi);
		CorePrint ("    cs:eip = 0x%04x:0x%08x\n", cs & 0xffff, eip);
		CorePrint ("    ebx = 0x%08x     edi = 0x%08x ", ebx, edi);
		CorePrint ("    ss:esp = 0x%04x:0x%08x\n", ss & 0xffff, esp);
		CorePrint ("    ecx = 0x%08x     ebp = 0x%08x ", ecx, ebp);
		CorePrint ("    ds = 0x%04x    fs = 0x%04x\n", ds & 0xffff, fs & 0xffff);
		CorePrint ("    edx = 0x%08x  eflags = 0x%08x ", edx, eflags);
		CorePrint ("    es = 0x%04x    gs = 0x%04x\n", es & 0xffff, gs & 0xffff);
	}
}
