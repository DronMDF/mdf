//
// Copyright (c) 2000-2010 ������ ������ <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Resource.h"

namespace Core {

class Custom: public Resource
{
public:
	static Resource *Create();
	virtual Custom *asCustom();

private:
	Custom();
	
	// ����������
	Custom(const Custom &);
	Custom & operator =(const Custom &);
};

} // namespace Core;
