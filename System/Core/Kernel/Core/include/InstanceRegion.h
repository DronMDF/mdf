//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Instance.h"

namespace Core {
	
class Resource;

class InstanceRegion : public Instance
{
private:
	offset_t m_position;	/// Позиция окна
	offset_t m_offset;	/// смещение области от начала родительской сущности
	size_t m_size;		/// размер области в родительской сущности
public:
	InstanceRegion(Resource *resource, uint32_t access, 
		       offset_t poffset, size_t psize, offset_t skip);
		       
	bool inBounds(laddr_t addr) const;
	const PageInstance *PageFault(laddr_t addr, uint32_t *access);
};

} // namespace Core
