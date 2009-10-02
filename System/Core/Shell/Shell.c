//
// Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
// All rights reserved.
//

#include <MDF/Kernel.h>
#include <MDF/Namer.h>

#include <stdio.h>
#include <string.h>

char id[] = "MDFVER: System-Core/Shell-" VERSION;

int getchar (void)
{
	char buf[256];
	union namer_message *msg = (union namer_message *)buf;

	while (1) {
		msg->Request.Offset =
			offsetof (union namer_message, Request.Request);
		strncpy (msg->Request.Request, "Keyboard://get",
			256 - offsetof(union namer_message, Request.Request));
		msg->Request.Size = strlen (msg->Request.Request) + 1;

		if (NamerCall (msg, msg->Request.Offset + msg->Request.Size, 0)
			!= KERNEL_OK)
		{
			continue;
		}

		if (msg->Reply.Status != NAMER_OK)
			continue;

		break;
	}

	int res = 0;
	strncpy ((char *)&res, msg->Reply.Reply, 3);

	return res;
}

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

int main (int argc, char **argv)
{
	// Поспим чуток, чтобы с клавиатурой не пересекаться.
	KernelSheduleNext (150);

	puts ("\n\n");

	while (1) {
		puts ("MDF $ ");

		while (1) {
			int ch = getchar ();
			ch &= 0x7f;

			if (ch == '\n') {
				puts ("\nUnknown command\n");
				break;
			}

			if (ch < ' ')
				continue;

			puts ((char *)&ch);
	}	}

	return 0;
}
