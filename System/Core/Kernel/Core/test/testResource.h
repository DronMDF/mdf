//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "../Resource.h"

class testResource : public Core::Resource
{
public:
	using Core::Resource::m_event;
};
