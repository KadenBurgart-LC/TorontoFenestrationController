#include "th_MechanicalActions.h"

#include "HAL.h"

namespace {
	Thread createTask_STOP_ALL(){
		Thread myThread;

		myThread.Active = false;
		myThread.RootMethod = th_MechanicalActions::t_STOP_ALL_tick;
		myThread.ReadyPeriod_ms = 100;

		myThread.IsTerminalTask = true;

		return myThread;
	}

	namespace StopAll {
		uint8_t StateTracker = 0;
		unsigned long StartTime = 0;
		unsigned long DelayThreshold = 2000;
	}
}

// Public members
namespace th_MechanicalActions {
	Thread task_STOP_ALL = createTask_STOP_ALL();

	int8_t t_STOP_ALL_tick(){
		int8_t returnCode = 0;

		if (StopAll::StateTracker == 0){
			StopAll::StartTime = millis();

			HAL::set_C0_1_RgbLed(8,0,0);

			StopAll::StateTracker++;
		}
		else if (StopAll::StateTracker == 1){
			// Wait for 1 second
			if ((unsigned long)(millis() - StopAll::StartTime) > StopAll::DelayThreshold) StopAll::StateTracker++;
		}
		else if (StopAll::StateTracker == 2){
			HAL::set_C0_1_RgbLed_ReadySignal();
			StopAll::StateTracker = 0;
			return 1;
		}

		return returnCode;
	}
}