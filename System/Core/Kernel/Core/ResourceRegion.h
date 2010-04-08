//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Resource.h"
#include "Memory.h"

namespace Core {

// Соглашение о именовании параметров:
// poffset - смещение родительской сущности (физической памяти, региона
//	или номер первого порта ввода-вывода)
// psize - размер подкючаемой области родительской сущности
// skip - расстояние от начала региона
	
class ResourceRegion : public Resource
{
protected:
	Memory *m_memory;
		
private:
	const size_t m_size;
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

	const PageInstance *CopyOnWrite(offset_t offset, const PageInstance *page);

protected:
	virtual Memory *getMemory();
	
	int bindRegion(ResourceRegion *parent, offset_t poffset, 
		       size_t psize, offset_t skip);
	int bindPhysical(paddr_t poffset, size_t psize, offset_t skip);
	
public:
	ResourceRegion(size_t size, uint32_t access);
	virtual ~ResourceRegion();

	virtual int Modify(int param_id, const void *param, size_t param_size);

	bool inBounds(laddr_t addr, laddr_t base = 0) const;

	const PageInstance *PageFault(offset_t offset, uint32_t *access);

	size_t size() const;
	offset_t offset() const;

	virtual bool copyIn(offset_t offset, const void *src, size_t size);
};

} // namespace Core
