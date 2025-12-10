#ifndef TH_TEST_H
#define TH_TEST_H

#include "lib_OSBos.h"
#include "HAL.h"
#include "Arduino.h"

namespace {
	
}

namespace th_MechanicalActions {
	extern Thread task_STOP_ALL;

	int8_t t_STOP_ALL_tick();
}

#endif