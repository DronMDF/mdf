//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

void StubClockHandler();
void StubMicroSleep(uint32_t us);
void StubTimeInit();

tick_t StubGetTimestampCounter();

