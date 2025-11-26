#include "th_Blink.h"

#include "HAL.h"

// Private members
namespace {
	unsigned int blinkHalfPeriod_ms = 1000;
	unsigned long lastAction_ms = 0;
	bool currentState = false;
}

// Public members
namespace th_Blink {
	void tick(){
		currentState = !currentState;

		HAL::setDigitalOutput(HAL::DigitalOutput::YELLOW_LED, currentState);
	}
}


