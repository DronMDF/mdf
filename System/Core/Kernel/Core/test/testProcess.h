//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

class testProcess : public Core::ResourceProcess
{
public:
	testProcess()
		: Core::ResourceProcess(0)
	{
		Register();
	}
};
