//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"
#include "CoreLocal.h"
#include "Resources.h"	// только для Core::FindResources

#include "Region.h"

namespace Core {

ResourceRegion::ResourceRegion(size_t size, uint32_t access)
	: m_memory(0),
	  m_size(size),
	  m_offset(0),
	  m_access(access),
	  m_binded(false),
	  m_parent(0),
	  m_parent_offset(0)
{
}

ResourceRegion::ResourceRegion(offset_t offset, size_t size, uint32_t access)
	: m_memory(0),
	  m_size(size),
	  m_offset(offset),
	  m_access(access),
	  m_binded(false),
	  m_parent(0),
	  m_parent_offset(0)
{
}

ResourceRegion::~ResourceRegion ()
{
	delete m_memory;
}

ResourceRegion *ResourceRegion::asRegion ()
{
	return this;
}

Memory *ResourceRegion::getMemory()
{
	if (m_memory == 0) {
		m_memory = new Memory(m_offset + m_size, Memory::ALLOC);
	}
	return m_memory;
}

// Схема физического биндинга
// memory		000000001111111122222222
// poffset		-----------|	  |
// psize			   +------+	<- область будет импортирована в регион
// m_offset		-----	   |	  |
// m_size		     ------+------+---	<- область региона
// skip			     ------|	  |	<- смещение в регионе

int ResourceRegion::bindPhysical(offset_t poffset, size_t psize, offset_t skip)
{
	if (m_binded) return ERROR_BUSY;
	if (poffset % PAGE_SIZE != (m_offset + skip) % PAGE_SIZE) return ERROR_UNALIGN;
	if (skip + psize > m_size) return ERROR_OVERSIZE;

	const int rv = getMemory()->bindPhysical(poffset, psize, m_offset + skip);
	if (rv == SUCCESS) m_binded = true;
	return rv;
}

// Схема регионного биндинга.
// memory		000000001111111122222222
// parent->m_offset	-----      |    |
// Parent->m_size	     ------+----+--
// poffset		     ------|	|
// psize			   +----+	<- Будет забиндена в регион
// m_offset		-----      |	|
// m_size		     ------+----+-----	<- область региона
// skip			     ------|	|	<- смещение в регионе.

int ResourceRegion::bindRegion(id_t parent, offset_t poffset, size_t psize, offset_t skip)
{
	if (m_binded) return ERROR_BUSY;

	Resource *resource = FindResource (parent);
	if (resource == 0)
		return ERROR_INVALIDID;

	m_parent = resource->asRegion();
	if (m_parent == 0)
		return ERROR_INVALIDID;

	// TODO: странное соглашение, ИМХО poffset надо сравнивать с m_size
	if (poffset < m_offset)
		return ERROR_INVALIDPARAM;

	// TODO: И это тоже странное соглашение. Надо перепахивать.
	m_parent_offset = poffset - m_offset;
	
	STUB_ASSERT (m_parent_offset % PAGE_SIZE != 0, "Unaligned parent region");
	STUB_ASSERT (skip + psize > m_size, "Overload region");
	STUB_ASSERT (m_parent_offset + psize > m_parent->m_size, "Small parent region");

	// И все... подкачка будет работать с родительского региона.
	// TODO: только я что-то shift нигде не зафиксировал.

	m_binded = true;
	return SUCCESS;
}

int ResourceRegion::Modify (int param_id, const void *param, size_t param_size)
{
	const KernelModifyRegionBindParam *bindparam = 0;
	if (param_size == sizeof (KernelModifyRegionBindParam))
		bindparam = reinterpret_cast<const KernelModifyRegionBindParam *>(param);

	switch (param_id) {
		case RESOURCE_MODIFY_REGION_PHYSICALBIND:
			if (bindparam == 0)
				return ERROR_INVALIDPARAM;

			return bindPhysical(bindparam->offset, bindparam->size, bindparam->shift);

		case RESOURCE_MODIFY_REGION_REGIONBIND:
			if (bindparam == 0)
				return ERROR_INVALIDPARAM;

			return bindRegion(bindparam->id, bindparam->offset, bindparam->size, bindparam->shift);

		default:
			break;
	}

	return Resource::Modify(param_id, param, param_size);
}

bool ResourceRegion::inBounds(laddr_t addr, laddr_t base) const
{
	STUB_ASSERT (base % PAGE_SIZE != 0, "Region is not aligned");
	
	if (addr < base + m_offset) return false;
	if (base + m_offset + m_size <= addr) return false;
	return true;
}

const PageInstance *ResourceRegion::CopyOnWrite(offset_t offset, const PageInstance *page)
{
	PageInfo *pp = StubGetPageByInstance(page);
	STUB_ASSERT (pp == 0, "Missing parent page");

	laddr_t pa = StubPageTemporary(pp);
	STUB_ASSERT (pa == 0, "Missing parent page temporary addr");

	if (offset < PAGE_SIZE) {
		// Первая страница копируется не полностью
		getMemory()->copyIn(m_offset, reinterpret_cast<void *>(pa + m_offset),
			PAGE_SIZE - m_offset);
	} else {
		// Последняя тоже не полностью, но за этим проследит Memory
		getMemory()->copyIn(offset & ~(PAGE_SIZE - 1),
				reinterpret_cast<void *>(pa), PAGE_SIZE);
	}

	StubPageUntemporary(pp);
	return getMemory()->getPage(offset);
}

// TODO: При биндинге на регионы необходимо указывать еще и права доступа
//       Это позволить делать окна на существующие регионы с соответствующими
//	 правами

const PageInstance *ResourceRegion::PageFault(offset_t offset, uint32_t *access)
{
	if (offset < m_offset)
		return 0;

	*access &= m_access;

	// Сперва необходимо проверить есть ли страница в этом регионе.
	// Если есть - отдаем ее.
	const PageInstance *page = getMemory()->getPage(offset);
	if (page != 0)
		return page;

	// Если есть родительский регион - пробуем вытянуть с него.
	if (m_parent != 0) {
		uint32_t paccess = *access;
		page = m_parent->PageFault(m_parent_offset + offset, &paccess);

		if (page != 0) {
			// Проверить доступ на запись, есть родитель такового не дает,
			// а он нам нужен, то темпорарим обе страницы и копируем.
			if (isSet(*access, RESOURCE_ACCESS_WRITE) &&
				!isSet(paccess, RESOURCE_ACCESS_WRITE))
			{
				page = CopyOnWrite (offset, page);
			}

			if (page != 0)
				return page;
		}
	}

	// А если родительского нету - то делаем свой PageFault;
	return getMemory()->PageFault(offset);
}

offset_t ResourceRegion::offset() const
{
	return m_offset;
}

/// Чистый размер региона
size_t ResourceRegion::size() const
{
	return m_size;
}

bool ResourceRegion::copyIn(offset_t offset, const void *src, size_t size)
{
	if (offset < m_offset) return false;
	return getMemory()->copyIn(offset, src, size);
}

} // namespace Core
