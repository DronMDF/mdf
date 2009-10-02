//
// Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
// All rights reserved.
//

#pragma once

// По стандарту bool должен преобразоваться в _Bool не знаю нафиг им это понадобилось.
#define bool	int
#define false	0
#define true	1

#define __bool_true_false_are_defined	1
