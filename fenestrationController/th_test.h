#ifndef TH_TEST_H
#define TH_TEST_H

#include <OSBos.h>      // The custom library that acts as our simple kernel https://github.com/actuvon/OSBos
#include "HAL.h"
#include "Arduino.h"

namespace {
	inline uint8_t StateTracker = 0;
	inline unsigned long StartTime = 0;

	inline unsigned long DelayThreshold = 2000;
}

namespace th_test {

	inline int8_t tick(){
		int8_t returnCode = 0;

		if (StateTracker == 0){
			StartTime = millis();

			HAL::set_C0_1_RgbLed(0,3,8);

			StateTracker++;
		}
		else if (StateTracker == 1){
			// Wait for 1 second
			if ((unsigned long)(millis() - StartTime) > DelayThreshold) StateTracker++;
		}
		else if (StateTracker == 2){
			HAL::set_C0_1_RgbLed_ReadySignal();
			StateTracker = 0;
			return 1;
		}

		return returnCode;
	}

	inline Thread createMyThread(){
		Thread myThread;

		myThread.Active = false;
		myThread.RootMethod = tick;
		myThread.ReadyPeriod_ms = 300;

		myThread.IsTerminalTask = true;

		return myThread;
	}

	inline Thread thread = createMyThread();
}

#endif