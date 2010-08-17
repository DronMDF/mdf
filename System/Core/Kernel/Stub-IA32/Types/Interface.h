//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Согшашение о вызове...
// Все параметры передаются строго по порядку в следующих регистрах:
// eax ebx ecx edx

inline
int KernelWait (id_t id, int event_id, timeout_t timeout)
{
	int rv = 0;

	__asm__ __volatile__ (
		"int	$0x30"
		: "=a" (rv)
		: "a" (id), "b" (event_id), "c" (timeout)
	);

	return rv;
}

inline
int KernelFind (const char *name, size_t name_size, id_t * const id)
{
	int rv = 0;

	__asm__ __volatile__ (
		"int	$0x31"
		: "=a" (rv)
		: "a" (name), "b" (name_size), "c" (id)
		: "memory"
	);

	return rv;
}

inline
int KernelCreate (int type, const void *param, size_t param_size, id_t *id)
{
	int rv = 0;

	__asm__ __volatile__ (
		"int	$0x32"
		: "=a" (rv)
		: "a" (type), "b" (param), "c" (param_size), "d" (id)
		: "memory"
	);

	return rv;
}

inline
int KernelCall (id_t id, void *buffer, size_t buffer_size,
				  int flags)
{
	int rv = 0;

	__asm__ __volatile__ (
		"int	$0x33"
		: "=a" (rv)
		: "a" (id), "b" (buffer), "c" (buffer_size), "d" (flags)
		: "memory"
	);

	return rv;
}

inline
int KernelAttach (id_t resource_id, id_t process_id,
				    int access, unsigned long specific)
{
	int rv = 0;

	__asm__ __volatile__ (
		"int	$0x34"
		: "=a" (rv)
		: "a" (resource_id), "b" (process_id), "c" (access), "d" (specific)
	);

	return rv;
}

inline
int KernelDetach (id_t id, int flags)
{
	int rv = 0;

	__asm__ __volatile__ (
		"int	$0x35"
		: "=a" (rv)
		: "a" (id), "b" (flags)
	);

	return rv;
}

inline
int KernelModify (id_t id, int param_id, const void *params, size_t param_size)
{
	int rv = 0;

	__asm__ __volatile__ (
		"int	$0x36"
		: "=a" (rv)
		: "a" (id), "b" (param_id), "c" (params), "d" (param_size)
		: "memory"
	);

	return rv;
}

inline
int KernelInfo (id_t id, int info_id, void *info, size_t *info_size)
{
	int rv = 0;

	__asm__ __volatile__ (
		"int	$0x37"
		: "=a" (rv)
		: "a" (id), "b" (info_id), "c" (info), "d" (info_size)
		: "memory"
	);

	return rv;
}

#ifdef __cplusplus
}
#endif

static inline
void write_io_byte(uint16_t port, uint8_t value)
{
	__asm__ __volatile__ (
		"outb %b0, %w1" : :
		"a"(value), "Nd"(port));
}

static inline
uint8_t read_io_byte(uint16_t port)
{
        int value;
        __asm__ __volatile__ (
		"inb %w1, %b0"
		: "=a"(value)
		: "Nd" (port));
        return value;
}

static inline
void lock(lock_t *lock)
{
	lock_t oldval = 666;
	do {
		__asm__ __volatile__ ( "xchgl %0, %1"
                	:"=q" (oldval), "=m" (*lock)
                	:"0" (oldval) : "memory" );
	} while (oldval != 0);
}

static inline
void unlock(lock_t *lock)
{
	*lock = 0;
}
