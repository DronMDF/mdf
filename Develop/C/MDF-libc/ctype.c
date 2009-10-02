//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

int isdigit(int c)
{
	return (c >= '0' && c <= '9') ? 1 : 0;
}

int isspace(int c)
{
	return (c == ' ' || c == '\t' || c == '\f' || c == '\n' || c == '\r' || c == '\v') ? 1 : 0;
}
