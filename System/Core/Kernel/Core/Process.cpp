//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"
#include "Stub.h"
#include "Core.h"

#include "List.h"
#include "Memory.h"
#include "Resource.h"
#include "ResourceInstance.h"
#include "Region.h"
#include "Process.h"
#include "Thread.h"

#include "Resources.h"	// TODO: Надо поделить на классы.

namespace Core {

ResourceProcess::ResourceProcess (laddr_t entry)
	: m_entry(entry),
	  m_instance_list(&ResourceInstance::ProcessLink),
	  m_pagetable(USER_PAGETABLE_SIZE, Memory::ALLOC | Memory::ZEROING)
{
	// TODO: Все способности в инстанции доступны
	ResourceInstance *self_instance = this->CreateInstance(RESOURCE_ACCESS_OWNER);

	m_instance_list.Insert (self_instance);
}

ResourceProcess::~ResourceProcess ()
{
	for (ResourceInstance *instance = m_instance_list.getFirst(); instance != 0; )
	{
		ResourceInstance *instance_to_delete = instance;
		instance = m_instance_list.getNext (instance);

		if (instance_to_delete->getResource() != this) {
			m_instance_list.Remove(instance_to_delete);
			delete instance_to_delete;
		}
	}

	STUB_ASSERT(m_instance_list.getSize() != 1, "Missing selfprocess instance");
	m_instance_list.Remove(m_instance_list.getFirst());
}

ResourceProcess *ResourceProcess::asProcess ()
{
	return this;
}

ResourceThread *ResourceProcess::Call ()
{
	// TODO: Данный вызов возможен только на неинициализированном процессе.
	// Если в процессе уже есть нити или коллы, то данный вызов возвращает 0.

	ResourceThread *thread = new ResourceThread(this, m_entry);
	STUB_ASSERT(thread == 0, "Unable to alloc thread");

	thread->Register();

	// TODO: Может быть совместить создание и аттач?
	int rv = Attach(thread, RESOURCE_ACCESS_OWNER, 0);
	STUB_ASSERT(rv != SUCCESS, "Unable to attach thread");

	return thread;
}

bool ResourceProcess::CheckRegionPlace (const ResourceRegion *region, laddr_t base) const
{
	// Границы подгоняем до границ страниц
	const laddr_t lowbound = base & PADDR_MASK;
	const laddr_t hibound = (base + region->size() + PAGE_SIZE - 1) & PADDR_MASK;

	// Нижняя граница пользовательской памяти
	if (lowbound < USER_MEMORY_BASE + PAGE_SIZE)
		return false;

	// Верхняя граница пользовательской памяти
	if (hibound >= USER_STACK_BASE - PAGE_SIZE)
		return false;

	// Проверяем на пересечение с имеющимися регионами.
	for (ResourceInstance *instance = m_instance_list.getFirst();
		instance != 0;
		instance = m_instance_list.getNext (instance))
	{
		Resource *resource = instance->getResource();
		ResourceRegion *exregion = resource->asRegion();

		if (exregion == 0)
			continue;

		laddr_t raddr = instance->getAddr();
		if (hibound <= raddr)
			continue;

		if (base >= raddr + exregion->size())
			continue;

		return false;
	}

	return true;
}

// ubase - хранит базу в пространстве пользователя
laddr_t ResourceProcess::selectRegionBase (const ResourceRegion *region, uint32_t ubase) const
{
	STUB_ASSERT (region == 0, "setBase for non region");

	if (ubase == 0) {
		// Адрес не определен - определяем
		for (laddr_t base = 0; ; base = 0) {
			base = (CoreRandom() & PADDR_MASK) + region->offset();

			if (CheckRegionPlace (region, base))
				return base;
		}
	}

	// Адрес определен - проверим.
	if (ubase % PAGE_SIZE != region->offset())
		return 0;

	laddr_t base = USER_MEMORY_BASE + ubase;
	if (!CheckRegionPlace (region, base))
		return 0;

	return base;
}

// TODO: Эта база - юзерлевела... надо бы переделать в кернеллевел..
int ResourceProcess::Attach (Resource *resource, int access, uint32_t ubase)
{
	// Ищем данный ресурс среди имеющихся инстанций
	for (ResourceInstance *instance = m_instance_list.getFirst();
		instance != 0;
		instance = m_instance_list.getNext (instance))
	{
		if (instance->getId() == resource->getId()) {
			STUB_FATAL ("Resource already attached");
			return ERROR_BUSY;
		}
	}

	laddr_t base = 0;
	if (resource->asRegion() != 0 && ubase != 0) {
		base = selectRegionBase (resource->asRegion(), ubase);
	}

	ResourceInstance *instance = resource->CreateInstance (access, base);
	STUB_ASSERT (instance == 0, "Unable to create instance");

	m_instance_list.Insert (instance);
	return SUCCESS;
}

int ResourceProcess::Detach(Resource *resource)
{
	for (ResourceInstance *instance = m_instance_list.getFirst();
		instance != 0; instance = m_instance_list.getNext(instance))
	{
		if (instance->getId() == resource->getId()) {
			m_instance_list.Remove(instance);
			delete instance;
			return SUCCESS;
		}
	}

	return ERROR_INVALIDID;
}

const PageInstance *ResourceProcess::PageFault (laddr_t addr, uint32_t *access)
{
	if (m_pagetable.inBounds(USER_PAGETABLE_BASE, addr))
	{
		*access &= RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE;
		return m_pagetable.PageFault(addr - USER_PAGETABLE_BASE);
	}

	for (ResourceInstance *instance = m_instance_list.getFirst();
		instance != 0;
		instance = m_instance_list.getNext (instance))
	{
		if (instance->inBounds(addr)) {
			return instance->PageFault(addr, access);
		}
	}

	CorePrint ("Unhandled pagefault in process 0x%8x at 0x%8x, mode 0x%3x\n",
		getId(), addr, *access);

	*access = 0;
	return 0;
}


ResourceInstance *ResourceProcess::FindInstance (id_t id) const
{
	for (ResourceInstance *instance = m_instance_list.getFirst();
		instance != 0;
		instance = m_instance_list.getNext (instance))
	{
		if (instance->getId() == id)
			return instance;
	}

	return 0;
}

int ResourceProcess::ModifyResource(id_t id, int param_id, const void *param, size_t param_size)
{
	Core::ResourceInstance *instance = FindInstance(id);
	if (instance == 0) return ERROR_INVALIDID;

	if (param_id == RESOURCE_MODIFY_REGION_MAP) {
		// Устанавливаем точку подключения региона.
		if (param_size != sizeof(laddr_t)) return ERROR_INVALIDPARAM;

		if (instance->getAddr() != 0) return ERROR_BUSY;

		// TODO: Надо выносить в функцию
		Resource *resource = instance->getResource();
		STUB_ASSERT (resource == 0, "No resource in instance");

		if (resource->asRegion() == 0) return ERROR_INVALIDID;

		uint32_t ubase = *reinterpret_cast<const uint32_t *>(param);

		laddr_t base = selectRegionBase(resource->asRegion(), ubase);
		if (base == 0) return ERROR_INVALIDPARAM;

		instance->setAddr(base);
		return SUCCESS;
	}

	return instance->Modify(param_id, param, param_size);
}

bool ResourceProcess::copyIn(offset_t offset, const void *src, size_t size)
{
	for (ResourceInstance *instance = m_instance_list.getFirst();
		instance != 0;
		instance = m_instance_list.getNext (instance))
	{
		if (!instance->inBounds(offset)) continue;
		
		// TODO: Всякая вот эта вот рутира просится в ResourceInstance
		Resource *resource = instance->getResource();
		ResourceRegion *region = resource->asRegion();
		if (region == 0) continue;

		const laddr_t limit = instance->getAddr() + region->offset() + region->size();
		size_t validsize = size;

		if (offset + size > limit) {
			// В этот регион все не влезает - пусть найдет место для хвоста.
			validsize = limit - offset;
			if (!copyIn(limit, static_cast<const char *>(src) + validsize, size - validsize)) {
				return false;
			}
		}

		return region->copyIn(offset - instance->getAddr(), src, validsize);
	}

	return false;
}

} // namespace Core
