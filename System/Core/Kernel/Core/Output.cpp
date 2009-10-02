//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "CoreLocal.h"

namespace Core {

static
void PrintString (const char * const string)
{
	for (int i = 0; string[i] != '\0'; ++i)
		StubPrintChar (string[i]);
}

static
void PrintNum (const uint64_t n, const unsigned int base, const int minlen)
{
	STUB_ASSERT (base < 2 || base > 16, "Illegal number base");

	if (base <= n || 1 < minlen) {
		PrintNum (n / base, base, minlen - 1);
	}

	StubPrintChar ("0123456789ABCDEF"[n % base]);
}

static
void PrintByte (const uint64_t value)
{
	static const char * const bip[] =
		{ "b", "KiB", "MiB", "GiB" };

	uint64_t bv = value;
	int v = 0;
	while (bv >= 1024 * 10) {
		bv /= 1024;
		v++;
	}

	PrintNum (bv, 10, 1);
	PrintString (bip[v]);
}

} // namespace Core

// Поддерживает следующие форматы: %s, %[width][l](n|x|b)

extern "C"
void CorePrint (const char *format, ...)
{
	__builtin_va_list args;
	__builtin_va_start(args, format);

	for (; *format != '\0'; format++) {
		if (*format != '%') {
			StubPrintChar (*format);
			continue;
		}

		++format;
		if (*format == '%') {
			StubPrintChar ('%');
			continue;
		}

		// Ширина вывода действует только на цифры...
		int width = 0;
		while ('0' <= *format && *format <= '9') {
			width *= 10;
			width += *format - '0';
			++format;
		}

		if (width == 0) {
			width = 1;
		}

		// Размер аргумента..
		bool long_arg = false;
		if (*format == 'l') {
			long_arg = true;
			++format;
		}

		if (*format == 's') {
			STUB_ASSERT (width > 1, "Width in %s modifiers");
			STUB_ASSERT (long_arg, "Long long %s argument");
			const char *str = __builtin_va_arg(args, const char *);
			Core::PrintString (str);
			continue;
		}

		const uint64_t num = long_arg ?
			__builtin_va_arg(args, uint64_t) :
			__builtin_va_arg(args, uint32_t);

		switch (*format) {
			case 'd':
			case 'u':
				Core::PrintNum (num, 10, width);
				break;

			case 'x':
				Core::PrintNum (num, 16, width);
				break;

			case 'b':
				STUB_ASSERT (width > 1, "Width in %b modifiers");
				Core::PrintByte (num);
				break;

			default:
				STUB_FATAL("Invalid character in ouput format string");
		}
	}

	__builtin_va_end(args);
}
