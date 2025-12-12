#include "MechanicalSystem.h"

#include "HAL.h"

namespace {
	// The fenestration wall will refuse to attempt to reach pressures higher than this
	const float MAX_TARGET_PRESSURE_PA = 1000;

	float _targetPressure = 0; // The pressure that the fenestration wall will try to get to
}

// Public members
namespace MechanicalSystem {
    /* Get or Set the target pressure for the fenestration wall in Pa.
	   The Set function will return true if the command worked or false if it did not.
	   This is the value that the PID loop will work towards attaining. */
	bool SetTargetPressure(float pressure){
		if( (MAX_TARGET_PRESSURE_PA * -1) <= pressure && pressure < MAX_TARGET_PRESSURE_PA){
			_targetPressure = pressure;
		}
		else _targetPressure = 0;

		return _targetPressure == pressure;
	}
	float GetTargetPressure(){ return _targetPressure; }

	namespace tk_StopAll {
		static Thread _createTask(){
			Thread myThread;

			myThread.Active = false;
			myThread.RootMethod = Tick;
			myThread.ReadyPeriod_ms = 100;

			myThread.IsTerminalTask = true;

			return myThread;
		}
		static uint8_t _threadState = 0;
		static unsigned long _startTime = 0;
		static unsigned long _delayThreshold = 2000;


		Thread Task = _createTask();


		int8_t Tick(){
			int8_t returnCode = 0;

			if (_threadState == 0){
				_startTime = millis();

				HAL::set_C0_1_RgbLed(8,0,0);

				_threadState++;
			}
			else if (_threadState == 1){
				// Wait for 1 second
				if ((unsigned long)(millis() - _startTime) > _delayThreshold) _threadState++;
			}
			else if (_threadState == 2){
				HAL::set_C0_1_RgbLed_ReadySignal();
				_threadState = 0;
				return 1;
			}

			return returnCode;
		}
	}
}