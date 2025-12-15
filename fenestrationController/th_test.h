#ifndef TH_TEST_H
#define TH_TEST_H

#include <OSBos.h>      // The custom library that acts as our simple kernel https://github.com/actuvon/OSBos
#include "HAL.h"
#include "Arduino.h"

namespace {
	// You can put stuff in here and it will only be visible to this file
}

namespace th_test {

	inline int8_t tick(){
		static uint8_t threadState = 0;
		static unsigned long startTime = 0;
		static unsigned long delayThreshold = 2000;

		int8_t returnCode = 0;

		if (threadState == 0){
			startTime = millis();

			HAL::set_C0_1_RgbLed(0,3,8);

			threadState++;
		}
		else if (threadState == 1){
			// Wait for 1 second
			if ((unsigned long)(millis() - startTime) > delayThreshold) threadState++;
		}
		else if (threadState == 2){
			HAL::set_C0_1_RgbLed_ReadySignal();
			threadState = 0;
			return 1;
		}

		return returnCode;
	}

	inline Thread thread = NewTerminalTask(tick, 300);

	// The old way of making an async terminal task before I made the NewTerminalTask(rootMethod, timer_ms) function...
	// inline Thread createMyThread(){
	// 	Thread myThread;

	// 	myThread.Active = false;
	// 	myThread.RootMethod = tick;
	// 	myThread.ReadyPeriod_ms = 300;

	// 	myThread.IsTerminalTask = true;

	// 	return myThread;
	// }
}

#endif