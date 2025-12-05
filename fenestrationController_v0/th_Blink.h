#ifndef TH_BLINK
#define TH_BLINK

#include <Arduino.h>
#include "lib_OSBos.h"

// Public members
namespace th_Blink {
	int8_t tick();

	inline Thread thread(tick, 1000);
}

#endif