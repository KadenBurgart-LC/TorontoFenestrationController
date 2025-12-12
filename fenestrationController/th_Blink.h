#ifndef TH_BLINK
#define TH_BLINK

#include <Arduino.h>
#include <OSBos.h>      // The custom library that acts as our simple kernel https://github.com/actuvon/OSBos

// Public members
namespace th_Blink {
	int8_t tick();

	inline Thread thread(tick, 1000);
}

#endif