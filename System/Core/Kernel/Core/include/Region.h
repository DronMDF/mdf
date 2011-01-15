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
	
class Region : public Resource
{
public:
	static Region *Create(const void *param, size_t param_size);

	// Конструктор используется в других тестах, потом его запрячем.
	Region(size_t size, uint32_t access);
	
	
protected:
	Memory *m_memory;
		
private:
	const size_t m_size;
	offset_t m_offset;

	uint32_t m_access;	// Ограничение ресурса на доступ.
	bool m_binded;		// Регион забинден

	Region *m_parent;
	offset_t m_parent_offset;

private:
	Region();
	Region(const Region &);
	Region & operator =(const Region &);

	virtual Region *asRegion ();

	const PageInstance *CopyOnWrite(offset_t offset, const PageInstance *page);

protected:
	virtual Memory *getMemory();
	
	int bindPhysical(paddr_t poffset, size_t psize, offset_t skip);
	
public:
	virtual ~Region();

	virtual int Modify(int param_id, const void *param, size_t param_size);

	bool inBounds(laddr_t addr, laddr_t base = 0) const;

	virtual const PageInstance *PageFault(offset_t offset, uint32_t *access);

	size_t size() const;
	offset_t offset() const;

	virtual bool copyIn(offset_t offset, const void *src, size_t size);

	int bindRegion(Region *parent, offset_t poffset, size_t psize, offset_t skip);
};

} // namespace Core
