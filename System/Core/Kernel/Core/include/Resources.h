//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Resource.h"

namespace Core {

// Чтобы не светить интерфейс хранилища ресурсов - вынесем одну функцию.
Resource *FindResource (const id_t id);

} // namespace Core
