//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Stub.h"

// Memory - Это хранилище определенного количества страниц.
// Имеет возможность (при вызове PageFault) аллоцировать страницы,
// При необходимости их нулит.

// До сих пор он был почти как регион, но я думаю его функцию следует упростить.
// Никаких смещений он знать не должен. Так же его не должны особо интересовать
// права доступа. Это все сильно изменит процедуры биндинга, они почти полностью
// переползут в регионы.

// Вопрос: А нужны ли мне вообще права доступа в инстанциях страниц?
//	На этом уровне все определяется правами регионов.

namespace Core {

class Memory
{
public:
	// Режимы работы блока памяти
	enum MODE {
		NONE = 0,
		ALLOC = 1,
		ZEROING = 2,
	};

private:
	// Массив страничных инстанций (ссылки)
	const PageInstance **m_page_instances;

	size_t m_size;		// Размер в байтах от смещения.
	uint32_t m_mode;	// Режим

private:
	// Конструкторы приватные, чтобы нельзя было конструировать его явно.
	Memory();
	Memory(const Memory &);
	Memory & operator =(const Memory &);

public:
	explicit Memory (size_t size, uint32_t mode = NONE);

	virtual ~Memory(void);

	const PageInstance *setPage(PageInfo *page, offset_t offset);
	const PageInstance *getPage(const offset_t offset) const;

	size_t getSize() const;

	virtual int bindPhysical(offset_t poffset, size_t size, offset_t skip);

	void Map(const void *src, size_t size, offset_t offset = 0);

	bool copyIn(offset_t offset, const void *src, size_t size);

	bool inBounds(laddr_t base, laddr_t addr, offset_t offset = 0) const;
	const PageInstance *PageFault(offset_t offset);
};

} // namespace Core
