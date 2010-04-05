//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Instance.h"

namespace Core {
	class Resource;

	class InstanceProcess : public Instance {
// 	private:
// 		InstanceProcess(const InstanceProcess &);
// 		InstanceProcess &operator =(const InstanceProcess &);
		
	public:
		explicit InstanceProcess(Resource *resource, uint32_t access)
			: Instance(resource, access, 0) {}
	};

} // namespace Core
