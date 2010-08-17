//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "CoreABI.h"

namespace std {

class type_info {
private:
	const char *m_name;

	type_info &operator =(const type_info &);
	type_info(const type_info &);

protected:
	explicit type_info(const char *n)
		: m_name(n)
	{
	}

public:
	virtual ~type_info()
	{
	}

	const char *name() const
	{
		return m_name;
	}
};

} // std namespace

namespace __cxxabiv1 {

// -----------------------------------------------------------------------------
// __fundamental_type_info используется в частности для int

class __fundamental_type_info : public std::type_info {
public:
	virtual ~__fundamental_type_info()
	{
	}
};

// -----------------------------------------------------------------------------

class __class_type_info : public std::type_info {
public:
	virtual ~__class_type_info()
	{
	}
};

class __si_class_type_info : public __class_type_info {
public:
	virtual ~__si_class_type_info()
	{
	}
};

class __vmi_class_type_info : public __class_type_info {
public:
	virtual ~__vmi_class_type_info();
};

// Почему-то при описании инлайн не генерируется ничего.
__vmi_class_type_info::~__vmi_class_type_info()
{
}

// -----------------------------------------------------------------------------

class __pointer_type_info : public std::type_info {
public:
	virtual ~__pointer_type_info()
	{
	}
};


} // namespace __cxxabiv1

// может и не нужна вовсе?
extern "C"
void StubRTTIInit()
{
}
