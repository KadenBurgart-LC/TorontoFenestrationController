#include "th_Blink.h"

#include "HAL.h"

// Private members
namespace {
	
}

// Public members
namespace th_Blink {
	int8_t tick(){
		static bool currentState = false;

		currentState = !currentState;

		HAL::setDigitalOutput(HAL::DigitalOutput::YELLOW_LED, currentState);

		return 0;
	}
}