//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Stub.h"

namespace Core {

template<typename T> class List;

template<typename T>
class Link {
private:
	friend class List<T>;

	T *m_prev;
	T *m_next;
	List<T> *m_list;
private:
	Link(const Link<T> &);
	Link<T> & operator =(const Link<T> &);

	void setPrev(T *prev);
	void setNext(T *next);
	void setList(List<T> *list);

	T *getPrev() const;
	T *getNext() const;
	const List<T> *getList() const;

public:
	Link();
	~Link();

	void Unlink(T *item);

	bool isLinked(const List<T> *list = 0) const;
};


template<typename T>
Link<T>::Link() : m_prev(0), m_next(0), m_list(0)
{
}

template<typename T>
Link<T>::~Link ()
{
	STUB_ASSERT (m_list != 0, "Destroy linked item");
	STUB_ASSERT (m_next != 0, "Destroy linked item");
	STUB_ASSERT (m_prev != 0, "Destroy linked item");
}

template<typename T>
T *Link<T>::getPrev() const
{
	return m_prev;
}

template<typename T>
T *Link<T>::getNext() const
{
	return m_next;
}

template<typename T>
const List<T> *Link<T>::getList() const
{
	return m_list;
}

template<typename T>
void Link<T>::setPrev(T *prev)
{
	STUB_ASSERT (m_list == 0, "Item not linked.");
	m_prev = prev;
}

template<typename T>
void Link<T>::setNext(T *next)
{
	STUB_ASSERT (m_list == 0, "Item not linked.");
	m_next = next;
}

template<typename T>
void Link<T>::setList(List<T> *list)
{
	STUB_ASSERT (list != 0 && m_list != 0, "Item already linked");
	m_list = list;
}

template<typename T>
void Link<T>::Unlink(T *item)
{
	if (m_list != 0) {
		m_list->Remove(item);
	}
}

template<typename T>
bool Link<T>::isLinked(const List<T> *list) const
{
	if (list == 0)
		return m_list != 0;

	return m_list == list;
}

} // namespace Core
