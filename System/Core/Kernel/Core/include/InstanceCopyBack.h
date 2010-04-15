//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Instance.h"
#include "Link.h"

namespace Core {
	
class Resource;

// TODO: Размер обратного копирования необходимо хранить. потому что
//	форвард может изменить размер запроса, но копибек останется
//	неизменным. значит размер необходимо передавать в конструкторе тоже.
//	а в copyIn выбрать меньшее из двух зол.

class InstanceCopyBack : public Instance {
private:
	laddr_t m_dst;
public:
	InstanceCopyBack(Resource *caller, laddr_t dst);
	
	void copyIn(const void *src, size_t size) const;
};

} // namespace Core
