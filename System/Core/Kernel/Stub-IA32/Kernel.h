//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

void StubInitKernel();
void StubKernelReservePages(paddr_t paddr, sizex_t size, laddr_t laddr, unsigned int flags);

unsigned int StubKernelPagesCnt();

void StubKernelUsePage(PageInfo *page, laddr_t laddr, unsigned int flags);
void StubKernelUnusePage(PageInfo *page);

void StubKernelDropMemory(laddr_t addr, size_t size);
