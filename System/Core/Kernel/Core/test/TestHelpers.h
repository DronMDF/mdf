//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

template<int callcount> 
class order_mock {
protected:
	mutable int m_order;
public:
	explicit order_mock() : m_order(0) {}
	~order_mock() { BOOST_CHECK_EQUAL(m_order, callcount); }

	void order_next() const { ++m_order; }
	void order(int n) const { BOOST_REQUIRE_EQUAL(++m_order, n); }
};

class visit_mock : private order_mock<1> {
public:
	void visit() const { m_order = 1; }
};
