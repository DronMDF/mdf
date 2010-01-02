//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

template<int callcount>
class order_mock {
private:
	mutable int m_order;
public:
	explicit order_mock() : m_order(0) {}
	~order_mock() { BOOST_CHECK_EQUAL(m_order, callcount); }

	void order_next() const { ++m_order; }
	void order(int n) const { BOOST_REQUIRE_EQUAL(++m_order, n); }
};

class visit_mock : private order_mock<1> {
public:
	void visit() const { order(1); }
};

static inline void fill_random (void *ptr, size_t size)
{
	char *p = static_cast<char *>(ptr);
	for (size_t i = 0; i < size; i++) {
		p[i] = rand() & 0xff;
	}
}
