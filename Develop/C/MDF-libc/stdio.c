//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "stdio.h"
#include "stdint.h"
#include "stdarg.h"
#include "stdbool.h"
#include "ctype.h"
#include "string.h"

// -----------------------------------------------------------------------------
// Вспомогательные функции

static inline
int uint2strn (char * restrict s, size_t n, unsigned int u)
{
	size_t p = 0;
	if (u == 0) {
		switch (n) {
			default:
				s[p++] = '0';
			case 1:
				s[p] = 0;
			case 0:
				break;
		}
		return p;
	}

	bool flag = false;

	for (unsigned int i = 1000000000; i > 0 && p < n - 1; u %= i, i /= 10) {
		if (u / i) {
			flag = true;
			s[p++] = u / i + '0';
		} else {
			if (flag)
			s[p++] = '0';
		}
	}

	s[p] = 0;
	return p;
}

// -----------------------------------------------------------------------------
// стандартные

int vsnprintf(char * restrict s, size_t n, const char * restrict format, va_list arg)
{
	// NOTE: В отличии от стандарта наше n будет указывать на размре буфера,
	// а строка при любом раскладе должна завершатся нулем.
	// но возвращаться будет длина строки.
	// то есть при достижении n функция вернет n - 1

	size_t p = 0;

	for (; *format && p < n - 1; format++) {
		if (*format != '%') {
			s[p++] = *format;
			continue;
		}

		format++;
		switch (*format) {
			case '%':
				s[p++] = '%';
				break;
			case 'c':
				s[p++] = va_arg(arg, int);
				break;
			case 's':
				strncpy(s + p,
				va_arg(arg, char *), (n - p) - 1);
				p += strlen(s + p);
				break;
			case 'u':
				p += uint2strn(s + p, n - p, va_arg(arg, unsigned int));
				break;
			default:
				return -1;
		}
	}

	s[p] = 0;
	return p;
}

int snprintf(char * restrict s, size_t n, const char * restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int rn = vsnprintf(s, n, format, args);
	va_end(args);
	return rn;
}

int sprintf(char * restrict s, const char * restrict format, ...)
{
	va_list args;
	va_start(args, format);
	int n = vsnprintf(s, SIZE_MAX, format, args);
	va_end(args);
	return n;
}

int sscanf(const char * restrict s, const char * restrict format, ...)
{
	va_list args;
	va_start(args, format);

	int ac = 0;

	unsigned int u;
	char *pos, *cp;

	for (; *format; ++format) {
		if (*format == *s) {
			s++;
			continue;
		}

		if (*format != '%')
			break;

		format++;
		switch (*format) {
			case 'u':
				for (; isspace (*s); s++);
				for (u = 0; isdigit (*s); s++)
					u = u * 10 + (*s - '0');

				*va_arg(args, unsigned int *) = u;
				ac++;
				break;
			case '[':
				format++;
				if (*format == '^') {
					// Исключения
					pos = (char *)s + strlen(s);
					for (format++; *format != ']'; format++) {
						cp = strchr (s, *format);
						if (cp != NULL && cp < pos)
							pos = cp;
					}
				} else {
					// Совпадения
					cp = pos = (char *)s;

					while (1) {
						for (; *format != ']'; format++) {
							while (*cp == *format)
								cp++;
						}

						if (cp == pos)
							break;
						pos = cp;
					}
				}

				strncpy (va_arg(args, char *), s, pos - s);
				ac++;
				s = pos;
				break;
			default:
				break;
		}
	}

	va_end(args);
	return ac;
}

int vsprintf(char * restrict s, const char * restrict format, va_list arg)
{
	return vsnprintf(s, SIZE_MAX, format, arg);
}
