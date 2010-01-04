//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"
#include "CoreLocal.h"
#include "Resources.h"	// только для Core::FindResources

#include "Region.h"

namespace Core {

// -----------------------------------------------------------------------------
// Базовый адрес региона должен быть выровнен на страницу, так будет логичнее
// А смещение это некоторое внутреннее понятие.

ResourceRegion::ResourceRegion (const struct KernelCreateRegionParam * const params)
	: m_memory(params->offset + params->size, Memory::ALLOC),
	  m_offset(params->offset),
	  m_access(params->access),
	  m_flags(0),
	  m_parent(0),
	  m_parent_offset(0)
{
}

ResourceRegion::ResourceRegion(offset_t offset, size_t size, uint32_t access)
	: m_memory(offset + size, Memory::ALLOC),
	  m_offset(offset),
	  m_access(access),
	  m_flags(0),
	  m_parent(0),
	  m_parent_offset(0)
{
}

ResourceRegion::~ResourceRegion ()
{
}

ResourceRegion *ResourceRegion::asRegion ()
{
	return this;
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

int ResourceRegion::ModifyBindPhysical (const KernelModifyRegionBindParam * const param)
{
	if (isSet(m_flags, REGION_BINDED)) {
		CorePrint ("Region is binded\n");
		return ERROR_BUSY;
	}

	if (!m_memory.PhysicalBind (param->offset, param->size, param->shift)) {
		CorePrint ("Memory not binded\n");
		return ERROR_BUSY;
	}

	m_flags |= REGION_BINDED;
	return SUCCESS;
}

// Схема регионного биндинга.

// Pages:		000000001111111122222222

// Parent:		ooooossssssssssssss
// Parent->m_offset:	-----
// Parent->m_size:	     --------------
// param->offset:	     ------
// param->size:			   --------

// Region:		ooooosssxxxPPPPPPPPxxx
// m_size:		     -----------------
// m_offset:		-----
// param->shift:	     ------

int ResourceRegion::ModifyBindRegion (const KernelModifyRegionBindParam * const param)
{
	if ((m_flags & REGION_BINDED) != 0)
		return ERROR_BUSY;

	Resource *resource = FindResource (param->id);
	if (resource == 0)
		return ERROR_INVALIDID;

	m_parent = resource->asRegion();
	if (m_parent == 0)
		return ERROR_INVALIDID;

	if (param->offset < m_offset)
		return ERROR_INVALIDPARAM;

	m_parent_offset = param->offset - m_offset;
	STUB_ASSERT (m_parent_offset % PAGE_SIZE != 0, "Unaligned parent region");
	STUB_ASSERT (m_offset + param->shift + param->size > m_memory.getSize(), "Overload region");
	STUB_ASSERT (m_parent_offset + m_offset + param->size > m_parent->m_memory.getSize(), "Small parent region");

	// И все... подкачка будет работать с родительского региона.
	// TODO: только я что-то shift нигде не зафиксировал.

	m_flags |= REGION_BINDED;

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

			return ModifyBindPhysical (bindparam);

		case RESOURCE_MODIFY_REGION_REGIONBIND:
			if (bindparam == 0)
				return ERROR_INVALIDPARAM;

			return ModifyBindRegion (bindparam);

		default:
			break;
	}

	return Resource::Modify(param_id, param, param_size);
}

bool ResourceRegion::inBounds (laddr_t addr, laddr_t base) const
{
	STUB_ASSERT (base % PAGE_SIZE != 0, "Region is not aligned");
	return m_memory.inBounds (base, addr, m_offset);
}

const PageInstance *ResourceRegion::CopyOnWrite(offset_t offset, const PageInstance *page)
{
	PageInfo *pp = StubGetPageByInstance(page);
	STUB_ASSERT (pp == 0, "Missing parent page");

	laddr_t pa = StubPageTemporary(pp);
	STUB_ASSERT (pa == 0, "Missing parent page temporary addr");

	if (offset < PAGE_SIZE) {
		// Первая страница копируется не полностью
		m_memory.Copy(reinterpret_cast<void *>(pa + m_offset),
			PAGE_SIZE - m_offset, m_offset);
	} else {
		// Последняя тоже не полностью, но за этим проследит Memory
		m_memory.Copy(reinterpret_cast<void *>(pa),
			PAGE_SIZE, offset & ~(PAGE_SIZE - 1));
	}

	StubPageUntemporary(pp);

	return m_memory.getPage(offset);
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
	const PageInstance *page = m_memory.getPage(offset);
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
	return m_memory.PageFault(offset);
}


offset_t ResourceRegion::getOffset() const
{
	return m_offset;
}

// Не совсем ясно что возвращать в качестве сайза...
// Может быть сайз от начала страницы?
size_t ResourceRegion::getSize() const
{
	return m_memory.getSize() - m_offset;
}

bool ResourceRegion::copyIn(offset_t offset, const void *src, size_t size)
{
	if (offset < m_offset) return false;
	return m_memory.copyIn(offset, src, size);
}

} // namespace Core
