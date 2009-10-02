//
// Copyright (c) 2000-2008 Валяев Андрей (dron@infosec.ru)
// All rights reserved.
//
// Created: 05/02/08 13:54:39
//

#include <iostream>
#include <iomanip>
#include <string>
using namespace std;

#include <stdarg.h>

#include <MDF/Stub-Linux.h>
#include <MDF/Core.h>

// DIEHARD runner
extern "C" void do_test(char *fn);

// Стаб принт делается на подобии IA-32шного...

void StubPrintByte (unsigned long value)
{
	static const char * const bip[] =
		{ "b", "KiB", "MiB", "GiB" };

	for (int v = 0; ; v++, value /= 1024) {
		if (value < 1024 * 10) {
			cout << dec << setw(1) << value << bip[v];
			break;
		}
	}
}

void StubPrint (const char *format, ...)
{
	va_list args;

	va_start (args, format);

	for (; *format != 0; format++) {
		if (*format != '%') {
			cout << *format;
			continue;
		}

		switch (*++format) {
			case '%':
				cout << '%';
				break;

			case 's':
				cout << va_arg (args, const char *);
				break;

			case 'u':
				cout << dec << setw(1) <<
					va_arg (args, unsigned long);
				break;

			case 'x':
				cout << hex << setw(8) << setfill('0') <<
					va_arg (args, unsigned long);
				break;

			case 'w':
				cout << hex << setw(4) << setfill('0') <<
					(va_arg (args, unsigned long) & 0xffff);
				break;

			case 'y':
				cout << hex << setw(2) << setfill('0') <<
					(va_arg (args, unsigned long) & 0xff);
				break;

			case 'b':
				StubPrintByte (va_arg (args, unsigned long));
				break;

			default:
				break;
		}
	}

	va_end(args);
}

static void StubHelp (const string &args)
{
	cout << "Nothing";
}

static void StubExit (const string &args)
{
	exit(0);
}

static void StubRandomTest (const string &args)
{
	// DIEHARD тест случайных чисел.
	do_test ("core");
}

struct _StubCmd {
	const char *cmdname;
	void (*cmdfunc)(const string &args);
};

static struct _StubCmd StubCmds[] = {
	{ "help", StubHelp },

	{ "exit", StubExit },
	{ "quit", StubExit },

	{ "diehard", StubRandomTest },
	{ "randomtest", StubRandomTest },

	{ 0, 0 },
};

void StubShell (void)
{
	while (true) {
		cout << endl << "MDF # ";

		string cmd;
		getline (cin, cmd);

		if (cmd.empty())
			continue;

		for (struct _StubCmd *cc = StubCmds; cc->cmdname != 0; cc++) {
			const string ccname(cc->cmdname);
			if (cmd.find (ccname) == 0) {
				cc->cmdfunc (string(cmd, ccname.length()));
				break;
			}
		}
	}
}

int main (int argc, char **argv)
{
	StubPrint ("Kernel      Stub-Linux-" VERSION " and %s\n", CoreVersion());
	CoreInit ();

	StubShell();

	return 0;
}

