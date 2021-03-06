//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/Kernel.h"
#include "include/CoreLocal.h"
#include "include/Resources.h"	// только для Core::FindResources

#include "include/Region.h"

using namespace Core;


Region *Region::Create(const void *param, size_t param_size)
{
	if (param == 0 || param_size != sizeof(KernelCreateRegionParam)) {
		return 0;
	}

	const KernelCreateRegionParam *region_param =
		reinterpret_cast<const KernelCreateRegionParam *>(param);

	return new Region(region_param->size, region_param->access);
}

Region::Region(size_t size, uint32_t access)
	: m_memory(0),
	  m_size(size),
	  m_offset(0),
	  m_access(access),
	  m_binded(false),
	  m_parent(0),
	  m_parent_offset(0)
{
}

Region::~Region ()
{
	delete m_memory;
}

Region *Region::asRegion ()
{
	return this;
}

Memory *Region::getMemory()
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

int Region::bindPhysical(paddr_t poffset, size_t psize, offset_t skip)
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

int Region::bindRegion(Region *parent, offset_t poffset, size_t psize, offset_t position)
{
	if (m_binded) return ERROR_BUSY;
	if (position + psize > m_size) return ERROR_INVALIDPARAM;
	if (poffset + psize > parent->size()) return ERROR_INVALIDPARAM;

	// TODO: Здесь не учитывается position!
	m_offset = (parent->offset() + poffset) % PAGE_SIZE;
	
	// Лучше прикапывать идентификатор, а то ресурс удалят и все, или вязать 
	// через инстанции (адрес инстанции можно использовать как смещение!)
	m_parent = parent;
	m_parent_offset = poffset;
	m_binded = true;
	
	return SUCCESS;
}

int Region::Modify (int param_id, const void *param, size_t param_size)
{
	// TODO: Здесь дублирование, но позже разберусь.
	if (param_id == RESOURCE_MODIFY_REGION_PHYSICALBIND) {
		if (param == 0 || param_size != sizeof(KernelModifyRegionBindParam)) {
			return ERROR_INVALIDPARAM;
		}

		const KernelModifyRegionBindParam *bindparam = 
			reinterpret_cast<const KernelModifyRegionBindParam *>(param);
	
		return bindPhysical(bindparam->offset, bindparam->size, 
				    bindparam->shift);
	}
	
	if (param_id == RESOURCE_MODIFY_REGION_REGIONBIND) {
		if (param == 0 || param_size != sizeof(KernelModifyRegionBindParam)) {
			return ERROR_INVALIDPARAM;
		}

		const KernelModifyRegionBindParam *bindparam = 
			reinterpret_cast<const KernelModifyRegionBindParam *>(param);
	
		Resource *resource = FindResource(bindparam->id);
		if (resource == 0) return ERROR_INVALIDID;
	
		Region *parent = resource->asRegion();
		if (parent == 0) return ERROR_INVALIDID;

		return bindRegion(parent, min(bindparam->offset, parent->size()),
				  bindparam->size, bindparam->shift);
	}

	return Resource::Modify(param_id, param, param_size);
}

bool Region::inBounds(laddr_t addr, laddr_t base) const
{
	STUB_ASSERT ((base - m_offset) % PAGE_SIZE != 0, "Region is not aligned");
	
	if (addr < base) return false;
	if (base + m_size <= addr) return false;
	return true;
}

const PageInstance *Region::CopyOnWrite(offset_t offset, const PageInstance *page)
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

const PageInstance *Region::PageFault(offset_t offset, uint32_t *access)
{
	*access &= m_access;

	// Сперва необходимо проверить есть ли страница в этом регионе.
	// Если есть - отдаем ее.
	const PageInstance *page = getMemory()->getPage(m_offset + offset);
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
	return getMemory()->PageFault(m_offset + offset);
}

offset_t Region::offset() const
{
	return m_offset;
}

size_t Region::size() const
{
	return m_size;
}

bool Region::copyIn(offset_t offset, const void *src, size_t size)
{
	STUB_ASSERT(size > m_size, "Overload region");
	return getMemory()->copyIn(m_offset + offset, src, size);
}
