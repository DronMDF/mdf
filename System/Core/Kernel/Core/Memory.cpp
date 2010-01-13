//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Memory.h"
#include "Stub.h"

#include "CoreLocal.h"

namespace Core {

Memory::Memory (size_t size, uint32_t mode)
	: m_page_instances(0),
	  m_size(size),
	  m_mode(mode)
{
}

Memory::~Memory ()
{
	if (m_page_instances == 0) return;

	const int page_count = (m_size + PAGE_SIZE - 1) / PAGE_SIZE;
	for (int pidx = 0; pidx < page_count; pidx++) {
		if (m_page_instances[pidx] == 0) continue;
		StubPageInstanceDelete(m_page_instances[pidx]);
	}

	delete[] m_page_instances;
}

const PageInstance *Memory::setPage (PageInfo *page, const offset_t page_offset)
{
	STUB_ASSERT (page_offset % PAGE_SIZE != 0, "Unaligned page in region");
	STUB_ASSERT (page_offset >= m_size, "Overload region");

	if (m_page_instances == 0) {
		const int page_count = (m_size + PAGE_SIZE - 1) / PAGE_SIZE;
		m_page_instances = new const PageInstance *[page_count];
		StubMemoryClear (m_page_instances, page_count * sizeof (PageInstance *));
	}

	const int pidx = page_offset / PAGE_SIZE;
	STUB_ASSERT(m_page_instances[pidx] != 0, "Page already exist");

	m_page_instances[pidx] = StubGetPageInstance (page, 0, 0);
	return m_page_instances[pidx];
}

const PageInstance *Memory::getPage (const offset_t offset) const
{
	STUB_ASSERT(offset >= m_size, "Big memory page offset");

	if (m_page_instances == 0)
		return 0;

	return m_page_instances[offset / PAGE_SIZE];
}

size_t Memory::getSize() const
{
	return m_size;
}

// offset и size всегда задают смещение и размер от родительской сущности.

// Схема физического биндинга
// Pages:		000000001111111122222222

// Phys:		ooooooooooossssssss
// param->offset:	-----------
// param->size:			   --------

// Region:		ooooossssssPPPPPPPPsss
// m_size		     -----------------
// m_offset:		-----
// param->shift:	     ------

// Здесь эта функция выглядит заметно проще.
bool Memory::PhysicalBind (paddr_t poffset, size_t size, offset_t shift)
{
	const offset_t loffset = shift;

	STUB_ASSERT (poffset % PAGE_SIZE != loffset % PAGE_SIZE, "Unaligned paddr for physical bind");
	STUB_ASSERT (shift + size > m_size, "Overload region");

	// TODO: В случае ошибки необходимо сделать откат занятых страниц в пул.

	for (offset_t coff = loffset & PADDR_MASK; coff < loffset + size; coff += PAGE_SIZE)
	{
		const paddr_t paddr = poffset - loffset + coff;
		PageInfo * const page = StubGetPageByPAddr(paddr);

		if (page == 0) {
			CorePrint ("No Page for paddr 0x%16lx\n", paddr);
			return false;
		}

		// TODO: проанализировать использование страницы.
		//	 для чего необходимы методы обхода PageInstances...

		if (!setPage (page, coff)) {
			CorePrint ("Unable to SetPage(0x%08x, 0x%08x)\n", page, coff);
			return false;
		}
	}

	return true;
}

void Memory::Map (const void *ptr, size_t size, offset_t offset)
{
	STUB_ASSERT(reinterpret_cast<laddr_t>(ptr) % PAGE_SIZE != offset % PAGE_SIZE,
		    "Unaligned mapping");
		    
	// TODO: Ну пока не мапим ничего.
	copyIn(offset, ptr, size);
}

bool Memory::inBounds (laddr_t base, laddr_t addr, offset_t offset) const
{
	STUB_ASSERT (base % PAGE_SIZE != 0, "Unaligned memory base");
	// offset здесь нужен исключительно для удобства вышестоящих ресурсов.
	STUB_ASSERT (offset >= PAGE_SIZE, "Very big offset");

	if (addr < base + offset)
		return false;

	if (base + m_size <= addr)
		return false;

	return true;
}

const PageInstance *Memory::PageFault(offset_t offset)
{
	const PageInstance *instance = getPage(offset);
	if (instance != 0)
		return instance;

	if (!isSet(m_mode, Memory::ALLOC))
		return 0;

	PageInfo *page = StubPageAlloc ();
	STUB_ASSERT (page == 0, "No free page");

	if (isSet(m_mode, Memory::ZEROING)) {
		// Очистить страницу.
		laddr_t paddr = StubPageTemporary (page);
		StubMemoryClear (reinterpret_cast<void *>(paddr), PAGE_SIZE);
		StubPageUntemporary (page);
	}

	instance = setPage (page, offset & ~(PAGE_SIZE - 1));
	STUB_ASSERT (instance == 0, "No page");

	return instance;
}

bool Memory::copyIn(offset_t offset, const void *src, size_t size)
{
	if (offset + size > m_size) return false;

	const size_t limit = min(m_size, size_t(offset) + size);
	const char *ssrc = static_cast<const char *>(src);
	for (offset_t soff = offset; soff < limit; ) {
		const offset_t poff = soff % PAGE_SIZE;
		const size_t psize = min(PAGE_SIZE - poff, limit - soff);
		
		const PageInstance *instance = PageFault(soff);
		if (instance == 0) return false;
		PageInfo *page = StubGetPageByInstance(instance);
		const laddr_t addr = StubPageTemporary(page);
		StubMemoryCopy (reinterpret_cast<void *>(addr + poff), ssrc, psize);
		StubPageUntemporary(page);

		soff += psize;
		ssrc += psize;
	}
	
	return true;
}

} // namespace Core
