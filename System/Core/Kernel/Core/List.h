//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Link.h"

namespace Core {
// Однонаправленные списки. Других пока нету, потом по ситуации заведу.

template<typename T>
class List
{
private:
	T *m_first;
	Link<T> T::* const m_field;

	List();
	List(const List<T> &);
	List<T> & operator = (const List<T> &);
public:
	List(Link<T> T::* field);
	~List ();

	T *getFirst () const;
	T *getNext (const T *item) const;
	int getSize () const;

	bool isEmpty() const;

	void Insert(T *item);
	void InsertAfter (T *item, T *afterit);
	void InsertBefore (T *item, T *beforeit);

	void Remove (T *item);
};

template<typename T>
List<T>::List(Link<T> T::* field)
	: m_first(0), m_field(field)
{
}

template<typename T>
List<T>::~List ()
{
	STUB_ASSERT (m_first != 0, "Destroy not empty list");
}

template<typename T>
T *List<T>::getFirst () const
{
	return m_first;
}

template<typename T>
T *List<T>::getNext (const T *item) const
{
	return (item->*m_field).getNext();
}

template<typename T>
void List<T>::Insert(T *item)
{
	(item->*m_field).setList(this);
	(item->*m_field).setNext(m_first);
	(item->*m_field).setPrev(0);

	if (m_first != 0) {
		(m_first->*m_field).setPrev(item);
	}

	m_first = item;
}

template<typename T>
void List<T>::InsertAfter (T *item, T *afterit)
{
	STUB_ASSERT ((afterit->*m_field).getList() != this, "item not in this list");

	(item->*m_field).setList(this);
	(item->*m_field).setNext(0);
	(item->*m_field).setPrev(afterit);

	if (T *next = (afterit->*m_field).getNext()) {
		(item->*m_field).setNext(next);

		STUB_ASSERT((next->*m_field).getList() != this, "Item not in this list");
		(next->*m_field).setPrev(item);
	}

	// Прямой порядок ломаем только здесь.
	(afterit->*m_field).setNext(item);
}

template<typename T>
void List<T>::InsertBefore (T *item, T *beforeit)
{
	STUB_ASSERT ((beforeit->*m_field).getList() != this, "item not in this list");

	(item->*m_field).setList(this);
	(item->*m_field).setNext(beforeit);
	(item->*m_field).setPrev(0);

	// Прямой порядок ломаем только здесь.
	if (T *prev = (beforeit->*m_field).getPrev()) {
		(item->*m_field).setPrev(prev);

		STUB_ASSERT((prev->*m_field).getList() != this, "Item not in this list");
		(prev->*m_field).setNext(item);
	} else {
		m_first = item;
	}

	(beforeit->*m_field).setPrev(item);
}

template<typename T>
void List<T>::Remove (T *item)
{
	STUB_ASSERT((item->*m_field).getList() != this, "Item not in this list");

	// Параноидальная проверка
	bool exists = false;
	for (const T *it = m_first; it != 0; it = getNext(it)) {
		if (it == item) {
			exists = true;
			break;
		}
	}
	STUB_ASSERT(!exists, "Item not in this list");

	// Удаление
	T *next = (item->*m_field).getNext();
	T *prev = (item->*m_field).getPrev();

	if (next != 0) {
		STUB_ASSERT((next->*m_field).getList() != this, "Item not in this list");
		(next->*m_field).setPrev(prev);
	}

	// Прямой порядок ломаем здесь
	if (prev != 0) {
		STUB_ASSERT((prev->*m_field).getList() != this, "Item not in this list");
		(prev->*m_field).setNext(next);
	} else {
		m_first = next;
	}

	(item->*m_field).setPrev(0);
	(item->*m_field).setNext(0);
	(item->*m_field).setList(0);
}

template<typename T>
int List<T>::getSize () const
{
	int size = 0;
	for (const T *item = m_first; item != 0; size++) {
		item = (item->*m_field).getNext();
	}
	return size;
}

template<typename T>
bool List<T>::isEmpty() const
{
	return m_first == 0;
}

} // namespace Core
