//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../List.h"
#include "../Memory.h"
#include "../Resource.h"
#include "../ResourceInstance.h"
#include "../Thread.h"
#include "../Process.h"
#include "../Core.h"
#include "../Kernel.h"

#include "testProcess.h"

using namespace std;
using namespace Core;

BOOST_AUTO_TEST_SUITE(process)

BOOST_AUTO_TEST_CASE(detach)
{
	// Удаление ресурсов всегда происходит через инстанцию. За исключением
	// процессов, с которыми немного иначе. С пассивными ресурсами все
	// просто, а нити удаляются через процесс, ссылка на который в них
	// хранится. Удаление происходит через вызов метода Detach. Процесс
	// обнаруживает инстанцию и удаляет ее. Ресур, обрнаружив удаление
	// последней инстанции удаляется сам.

	// Для тестирования детача необходимо к процессу подцепить ресурс через
	// инстанцию.

	bool deleted = false;

	testProcess process;

	class testResource : public Resource {
	private:
		bool *m_delete;

		testResource(const testResource &);
		testResource &operator =(const testResource &);
	public:
		testResource(bool *d) : Resource(), m_delete(d) {}
		~testResource() { *m_delete = true; }
	} *resource = new testResource(&deleted);

	resource->Register();
	process.Attach(resource, 0, 0);
	process.Detach(resource);
	BOOST_CHECK(deleted);
}

BOOST_AUTO_TEST_SUITE_END()
