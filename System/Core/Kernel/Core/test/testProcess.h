//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "../include/Process.h"

class testProcess : public Core::ResourceProcess
{
public:
	testProcess(laddr_t entry = 0)
		: Core::ResourceProcess(entry)
	{
		Register();
	}
};
