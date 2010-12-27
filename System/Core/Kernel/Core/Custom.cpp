//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

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
