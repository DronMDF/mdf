//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/Kernel.h"

#include "include/List.h"
#include "include/Memory.h"
#include "include/Resource.h"
#include "include/Process.h"
#include "include/Thread.h"
#include "include/CallPoint.h"

using namespace Core;

Resource *ResourceCall::Create(Process *process, const void *param, size_t size)
{
	if (process == 0) return 0;
	if (param == 0) return 0;
	if (size != sizeof(KernelCreateCallParam)) return 0;

	const KernelCreateCallParam *cp =
		reinterpret_cast<const KernelCreateCallParam *>(param);
	return new ResourceCall(process, cp->entry);
}

ResourceCall::ResourceCall(Process *process, laddr_t entry)
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

Thread *ResourceCall::Call()
{
	Thread *thread = new Thread(m_process, m_entry);
	STUB_ASSERT(thread == 0, "Unable to alloc thread");

	thread->Register();

	// TODO: Может быть совместить создание и аттач?
	int rv = m_process->Attach(thread, RESOURCE_ACCESS_OWNER, 0);
	STUB_ASSERT(rv != SUCCESS, "Unable to attach thread");

	return thread;
}
