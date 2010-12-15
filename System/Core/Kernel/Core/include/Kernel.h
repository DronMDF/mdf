//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

// Этот интерфейс платформенно независим.
// Для его крректности нужны только правильные типы от Stub.

enum KERNEL_ERROR {
	SUCCESS = 0,

	ERROR_NOTIMPLEMENT,
	ERROR_INVALIDID,
	ERROR_INVALIDPARAM,
	ERROR_BUSY,
	ERROR_NOMEMORY,
	ERROR_ACCESS,
	ERROR_UNALIGN,
	ERROR_OVERSIZE
};

enum KERNEL_RESOURCE_TYPE {
	RESOURCE_TYPE_REGION	= 1,
	RESOURCE_TYPE_PROCESS	= 2,
	RESOURCE_TYPE_THREAD	= 3,
	RESOURCE_TYPE_CALL	= 4,
	RESOURCE_TYPE_CUSTOM	= 5
};

// Флаги вызовов
enum KERNEL_RESOURCE_CALL_FLAGS {
	RESOURCE_CALL_ASYNC	= 0x00000001,
	RESOURCE_CALL_COPY	= 0x00000002,
	RESOURCE_CALL_READONLY	= 0x00000004,
	RESOURCE_CALL_FORWARD	= 0x00000008
};

// Права доступа к ядерным ресурсам
enum KERNEL_ACCESS {
	RESOURCE_ACCESS_READ	= 0x00000001,
	RESOURCE_ACCESS_WRITE	= 0x00000002,
	RESOURCE_ACCESS_CALL	= 0x00000004,
	RESOURCE_ACCESS_DESTROY = 0x00000008,
	RESOURCE_ACCESS_ATTACH	= 0x00000010,
	RESOURCE_ACCESS_DETACH	= 0x00000020,
	RESOURCE_ACCESS_MODIFY	= 0x00000040,
	RESOURCE_ACCESS_INFO	= 0x00000080,

	RESOURCE_ACCESS_OWNER	= 0x7fffffff
};

enum KERNEL_RESOURCE_MODIFY {
	RESOURCE_MODIFY_NAME	= 1,
	RESOURCE_MODIFY_DATA,
	RESOURCE_MODIFY_EVENT,

	RESOURCE_MODIFY_REGION = (RESOURCE_TYPE_REGION << 16),
	RESOURCE_MODIFY_REGION_REGIONBIND,
	RESOURCE_MODIFY_REGION_PHYSICALBIND,
	RESOURCE_MODIFY_REGION_MAP,
	RESOURCE_MODIFY_REGION_UNMAP,

	RESOURCE_MODIFY_PROCESS = (RESOURCE_TYPE_PROCESS << 16),
	
	RESOURCE_MODIFY_THREAD = (RESOURCE_TYPE_THREAD << 16),
	RESOURCE_MODIFY_THREAD_PRIORITY,

	RESOURCE_MODIFY_CALL = (RESOURCE_TYPE_CALL << 16),
	
	RESOURCE_MODIFY_CUSTOM = (RESOURCE_TYPE_CUSTOM << 16),
	RESOURCE_MODIFY_CUSTOM_IOBIND
};

enum KERNEL_RESOURCE_INFO {
	RESOURCE_INFO_STUB_VERSION = 1,
	RESOURCE_INFO_CORE_VERSION,

	RESOURCE_INFO_MEMORY,	// Статистика памяти
	RESOURCE_INFO_OWNER,	// Найти хозяина ресурса

	RESOURCE_INFO_REGION	= (RESOURCE_TYPE_REGION << 16),
	RESOURCE_INFO_REGION_INSTANCE_ADDR,	// Адрес региона в текущей инстанции.

	RESOURCE_INFO_PROCESS	= (RESOURCE_TYPE_PROCESS << 16),
	RESOURCE_INFO_THREAD	= (RESOURCE_TYPE_THREAD << 16),
	RESOURCE_INFO_THREAD_CURRENT,	// Идентификатор текущей нити.

	RESOURCE_INFO_CALL	= (RESOURCE_TYPE_CALL << 16),
	RESOURCE_INFO_CUSTOM	= (RESOURCE_TYPE_CUSTOM << 16)
};

enum KERNEL_THREAD_STATE {
	THREAD_ACTIVE,
	THREAD_WAIT
};

enum KERNEL_RESOURCE_EVENT {
	RESOURCE_EVENT_DESTROY = 1,

	RESOURCE_EVENT_REGION	= (RESOURCE_TYPE_REGION << 8),
	RESOURCE_EVENT_PROCESS	= (RESOURCE_TYPE_PROCESS << 8),
	RESOURCE_EVENT_THREAD	= (RESOURCE_TYPE_THREAD << 8),
	RESOURCE_EVENT_THREAD_EXIT,
	
	RESOURCE_EVENT_CALL	= (RESOURCE_TYPE_CALL << 8),

	RESOURCE_EVENT_USER	= 0x10000	// база пользовательских событий
};

#define INVALID_ID	0

struct KernelCreateRegionParam {
	size_t size;
	uint32_t access;	// ограничения на права региона
};

struct KernelCreateProcessParam {
	laddr_t entry;
};

struct KernelCreateThreadParam {
	laddr_t entry;
};

struct KernelCreateCallParam {
	laddr_t entry;
};

struct KernelCreateCustomParam {
	uint32_t reserved;
};

// Параметры биндинга регионов.
struct KernelModifyRegionBindParam {
	id_t id;
	size_t size;
	paddr_t offset;		// Смещение может быть за пределами физической памяти.
	laddr_t shift;
	uint32_t access;
};

struct KernelModifyCustomIoBindParam {
	uint32_t first;
	uint32_t last;
	uint32_t access;
};

// Информационные структуры
struct KernelInfoMemory {
	size_t MemoryTotal;
	size_t MemoryUsed;
	size_t KernelMemoryUsed;
	size_t KernelHeapTotal;
	size_t KernelHeapUsed;
};

// Программный интерфейс приложений.

#ifdef __cplusplus
extern "C" {
#endif

int KernelWait(id_t id, int event_id, timeout_t timeout);
int KernelFind(const char *name, size_t name_size, id_t *id);
int KernelCreate(int type, const void *param, size_t param_size, id_t *id);
int KernelCall(id_t id, void *buffer, size_t buffer_size, int flags);
int KernelAttach(id_t resource_id, id_t process_id, int access, uint32_t specific);
int KernelDetach(id_t id, int flags);
int KernelModify(id_t id, int param_id, const void *params, size_t param_size);
int KernelInfo(id_t id, int info_id, void *info, size_t *info_size);

#ifdef __cplusplus
}
#endif
