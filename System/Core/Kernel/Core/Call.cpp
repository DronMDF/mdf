//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"

#include "List.h"
#include "Memory.h"
#include "Resource.h"
#include "Call.h"
#include "Process.h"
#include "Thread.h"

namespace Core {

Resource *ResourceCall::Create(ResourceProcess *process, const void *param, size_t size)
{
	if (process == 0) return 0;
	if (param == 0) return 0;
	if (size != sizeof(KernelCreateCallParam)) return 0;

	const KernelCreateCallParam *cp =
		reinterpret_cast<const KernelCreateCallParam *>(param);
	return new ResourceCall(process, cp->entry);
}

ResourceCall::ResourceCall(ResourceProcess *process, laddr_t entry)
	: m_process(process), m_entry(entry)
{
}

ResourceCall *ResourceCall::asCall()
{
	return this;
}

laddr_t ResourceCall::getEntry() const
{
	return m_entry;
}

ResourceThread *ResourceCall::Call()
{
	ResourceThread *thread = new ResourceThread(m_process, m_entry);
	STUB_ASSERT(thread == 0, "Unable to alloc thread");

	thread->Register();

	// TODO: Может быть совместить создание и аттач?
	int rv = m_process->Attach(thread, RESOURCE_ACCESS_OWNER, 0);
	STUB_ASSERT(rv != SUCCESS, "Unable to attach thread");

	return thread;
}

} // namespace Core