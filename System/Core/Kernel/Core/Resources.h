//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Resource.h"

// TODO: Этот инклюд почти уже можно переименовывать в Custom.h

namespace Core {

// -----------------------------------------------------------------------------
// Конечные ресурсы

class ResourceCustom : public Resource
{
};

// Чтобы не светить интерфейс хранилища ресурсов - вынесем одну функцию.
Resource *FindResource (const id_t id);

} // namespace Core
