//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

void StubClockHandler();
void StubMicroSleep(uint32_t us);
void StubTimeInit();

tick_t StubGetTimestampCounter();

