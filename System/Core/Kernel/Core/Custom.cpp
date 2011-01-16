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

		const KernelModifyCustomIoBindParam *bind_param = 
			reinterpret_cast<const KernelModifyCustomIoBindParam *>(param);
	
		// TODO: Спросить у Стаба, можно ли заюзать этот диапазон 
		//	(если можно - бронируем паралельно)
		
		// Потом просто фиксируем параметры в структуре. чтобы по портфолту 
		//	кора могла оветить - наши порты или нет.
		
		// С другой стороны порты может контролировать сама кора, чтобы 
		//	 Стаб этим не нагружать. Тогда при инициализации стаб должен
		//	Уведомить кору о своем диапазоне портов (допустимом)
		
		return SUCCESS;
	}

	return Resource::Modify(param_id, param, param_size);
}
