//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "../include/Thread.h"

class testThread : public Core::ResourceThread
{
public:
	explicit testThread(Core::Process *process = 0)
		: Core::ResourceThread(process)
	{
		Register();
	}

	using Core::ResourceThread::m_txa;
	using Core::ResourceThread::m_txa_offset;
	using Core::ResourceThread::m_txa_access;
	
	using Core::ResourceThread::Kill;
};
