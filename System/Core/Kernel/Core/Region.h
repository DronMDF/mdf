//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Resource.h"
#include "Memory.h"

struct KernelCreateRegionParam;
struct KernelModifyRegionBindParam;

namespace Core {

// -----------------------------------------------------------------------------
// Конечные ресурсы

class ResourceRegion : public Resource
{
private:
	Memory 	m_memory;
	offset_t m_offset;

	uint32_t m_access;	// Ограничение ресурса на доступ.
	bool m_binded;		// Регион забинден

	ResourceRegion *m_parent;
	offset_t m_parent_offset;

private:
	ResourceRegion();
	ResourceRegion(const ResourceRegion &);
	ResourceRegion & operator =(const ResourceRegion &);

	virtual ResourceRegion *asRegion ();

	int ModifyBindPhysical (const KernelModifyRegionBindParam *param);
	int ModifyBindRegion (const KernelModifyRegionBindParam *param);

	const PageInstance *CopyOnWrite(offset_t offset, const PageInstance *page);

public:
	explicit ResourceRegion(const KernelCreateRegionParam *params);
	explicit ResourceRegion(offset_t offset, size_t size, uint32_t access);
	virtual ~ResourceRegion();

	virtual int Modify(int param_id, const void *param, size_t param_size);

	bool inBounds(laddr_t addr, laddr_t base = 0) const;

	const PageInstance *PageFault(offset_t offset, uint32_t *access);

	offset_t getOffset() const;
	size_t getSize() const;

	virtual bool copyIn(offset_t offset, const void *src, size_t size);
};

} // namespace Core
