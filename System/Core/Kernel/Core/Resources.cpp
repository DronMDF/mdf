//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "CoreLocal.h"
#include "Resources.h"
#include "ResourceStorage.h"

namespace Core {

// -----------------------------------------------------------------------------
// Инициализация подсистемы

void __init__ InitResources ()
{
}

// -----------------------------------------------------------------------------
// Ресурсонезависимые системные вызовы...

int ModifyIndependent (int param_id __unused__, const void *param __unused__, size_t param_size __unused__)
{
	return ERROR_INVALIDPARAM;
}

int InfoIndependent (int info_id __unused__, void *info __unused__, size_t *info_size __unused__)
{
	return ERROR_INVALIDPARAM;
}

// -----------------------------------------------------------------------------
// Утилиты

// Эта функция будет искать от текущего процесса, потом по глобальным инстанциям
// и под конец по глобальному списку ресурсов...
Resource *FindResource (const id_t id)
{
	// Но пока текущий процесс у нас не поддерживается. :)
	return ResourceStorage().Find(id);
}

} // namespace Core
