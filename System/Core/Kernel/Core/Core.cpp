//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/CoreLocal.h"

extern "C"
void __init__ CoreInit(void)
{
	Core::InitUtils();
	Core::InitResources();
	CorePrint ("Core Initialized.\n");
}
