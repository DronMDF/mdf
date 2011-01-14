//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/Kernel.h"
#include "include/Custom.h"

using namespace Core;

Resource *Custom::Create()
{
	return new Custom();
}

Custom::Custom()
{
}

Custom *Custom::asCustom()
{
	return this;
}

int Custom::Modify(int param_id, const void *param, size_t param_size)
{
	if (param_id == RESOURCE_MODIFY_CUSTOM_IOBIND) {
		if (param == 0) return ERROR_INVALIDPARAM;
		if (param_size != sizeof(KernelModifyCustomIoBindParam)) {
			return ERROR_INVALIDPARAM;
		}
		
		return SUCCESS;
	}

	return Resource::Modify(param_id, param, param_size);
}
