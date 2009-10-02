//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

// Основные понятия линков...

// list - список
// item - указатель на элемент
// link - поле по которому линкуется элемент

#define LIST(type)			\
	struct {			\
		type	*first;		\
	}

#define LISTITEM(type)			\
	struct {			\
		type	*next;		\
		type	*prev;		\
		LIST(type) *list;	\
	}


// Движение по списку вперед
#define ListItemNext(item, link)	\
	((item)->link.next)

// Движение по списку назад
#define ListItemPrev(item, link)	\
	((item)->link.prev)

// Указатель на список
#define ListItemList(item, link)	\
	((item)->link.list)


// Короче первым делом у нас идут двусвязные списки с указателем лист.

#define ListInit(list)			\
	do {				\
		(list).first = nullptr;	\
	} while (0);

// Первый элемент списка
#define ListItemFirst(list)	\
	(list).first

// Засунуть итем в голову списка
#define ListItemLink(itemlist, item, link)			\
	do {							\
		STUB_ASSERT ((item) == nullptr, "Null item");	\
		(item)->link.list = (void *)&(itemlist); 	\
		(item)->link.prev = nullptr;			\
		if (((item)->link.next = (itemlist).first))	\
			(itemlist).first->link.prev = (item);	\
		(itemlist).first = (item);			\
	} while (0);

// отвязать указанный итем от списка
#define ListItemUnlink(itemlist, item, link)			\
	do {							\
		STUB_ASSERT ((item) == nullptr, "Null item");	\
		STUB_ASSERT ((item)->link.list != (void *)&(itemlist), "Wrong item list"); \
		__typeof__ (item) __n = (item)->link.next;	\
		__typeof__ (item) __p = (item)->link.prev;	\
		if (__n != nullptr) __n->link.prev = __p;		\
		if (__p != nullptr) __p->link.next = __n;		\
		if (__p == nullptr) (itemlist).first = __n;	\
		(item)->link.list = nullptr;			\
	} while (0);
