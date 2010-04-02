//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

namespace Core {

	class Resource;

	class InstanceProcess {
	private:
		Resource *m_resource;
	public:
		explicit InstanceProcess(Resource *resource) : m_resource(resource) {}
		const Resource *resource() const { return m_resource; };
	};

} // namespace Core
