//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <stdarg.h>

#include <iostream>
#include <iomanip>

#include <Kernel.h>

using namespace std;

namespace {

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

	cout << setbase(10) << setw(1) << setfill('0') << bv;
	cout << bip[v];
}

}

extern "C" {

void CorePrint (const char *format, ...)
{
	va_list args;
	va_start(args, format);

	for (; *format != '\0'; format++) {
		if (*format != '%') {
			cout << *format;
			continue;
		}

		++format;
		if (*format == '%') {
			cout << *format;
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
			cout << va_arg(args, const char *);
			continue;
		}

		const uint64_t num =
			long_arg ? va_arg(args, uint64_t) : va_arg(args, uint32_t);

		switch (*format) {
			case 'd':
			case 'u':
				cout << setbase(10) << setw(width) << setfill('0') << num;
				break;

			case 'x':
				cout << setbase(16) << setw(width) << setfill('0') << num;
				break;

			case 'b':
				STUB_ASSERT (width > 1, "Width in %b modifiers");
				PrintByte (num);
				break;

			default:
				STUB_FATAL("Invalid character in ouput format string");
		}
	}

	va_end(args);
}

int CoreCreate(const Task *, int, const void *, size_t, id_t *)
{
	return SUCCESS;
}

int CoreModify(const Task *, id_t, int, const void *, size_t)
{
	return SUCCESS;
}

}