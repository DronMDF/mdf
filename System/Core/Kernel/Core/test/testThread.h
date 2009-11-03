//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "../Thread.h"

class testThread : public Core::ResourceThread
{
public:
	explicit testThread(Core::ResourceProcess *process = 0)
		: Core::ResourceThread(process)
	{
		Register();
	}
};
