#include "MechanicalSystem.h"

#include "HAL.h"

namespace {
	// The fenestration wall will refuse to attempt to reach pressures higher than this
	const float MAX_TARGET_PRESSURE_PA = 1000;

	float _targetPressure = 0; // The pressure that the fenestration wall will try to get to

	int8_t _lowPressureValveConfiguration = -1; // 0 means negative and 1 means positive. -1 means unknown. Anything else is an error.	
	int8_t _highPressureValveConfiguration = -1; // 0 means negative and 1 means positive. -1 means unknown. Anything else is an error.	
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

	int8_t GetLowPressureValveConfiguration() { return _lowPressureValveConfiguration; }
	int8_t GetHighPressureValveConfiguration() { return _highPressureValveConfiguration; }

	namespace tk_StopAll {
		Thread Task = NewTerminalTask(Tick, 200);

		int8_t Tick(){
			static uint8_t threadState = 0;
			static unsigned long startTime = 0;
			static unsigned long delayThreshold = 2000;

			int8_t returnCode = 0;

			if (threadState == 0){
				startTime = millis();

				HAL::set_C0_1_RgbLed(8,0,0);

				threadState++;
			}
			else if (threadState == 1){
				HAL::set_C0_1_RgbLed(8,0,0);

				HAL::setDigitalOutput(HAL::DigitalOutput::STRUCTURAL_BLOWER_POWER, false);
				HAL::setDigitalOutput(HAL::DigitalOutput::LEAKAGE_BLOWER_POWER, false);
				HAL::setDigitalOutput(HAL::DigitalOutput::WATER_PUMP_POWER, false);

				// Wait for 1 second
				if ((unsigned long)(millis() - startTime) > delayThreshold) threadState++;
			}
			else if (threadState == 2){
				HAL::set_C0_1_RgbLed_ReadySignal();
				threadState = 0;
				return 1; // Tell OSBos that the task is now finished
			}

			return returnCode;
		}
	}

	namespace tk_SetLowPressure_Positive {
		Thread Task = NewTerminalTask(Tick, 500);

		int8_t Tick(){
			static uint8_t threadState = 0;
			static unsigned long startTime = 0;
			static unsigned long delayThreshold = 2000;

			int8_t returnCode = 0;

			if (threadState == 0){
				startTime = millis();

				HAL::set_C0_1_RgbLed(8,0,8);

				_lowPressureValveConfiguration = -1; // The state is uncertain once this task begins

				threadState++;
			}
			else if (threadState == 1){
				HAL::set_C0_1_RgbLed(8,0,8);

				// Wait for 1 second
				if ((unsigned long)(millis() - startTime) > delayThreshold) threadState++;
			}
			else if (threadState == 2){
				_lowPressureValveConfiguration = 1;  // We can now garuntee that we are in the positive configuration

				HAL::set_C0_1_RgbLed_ReadySignal();
				threadState = 0;
				return 1; // Tell OSBos that the task is now finished
			}

			return returnCode;
		}
	}

	namespace tk_SetLowPressure_Negative {
		Thread Task = NewTerminalTask(Tick, 500);

		int8_t Tick(){
			static uint8_t threadState = 0;
			static unsigned long startTime = 0;
			static unsigned long delayThreshold = 2000;

			int8_t returnCode = 0;

			if (threadState == 0){
				startTime = millis();

				HAL::set_C0_1_RgbLed(8,0,8);

				_lowPressureValveConfiguration = -1; // The state is uncertain once this task begins

				threadState++;
			}
			else if (threadState == 1){
				HAL::set_C0_1_RgbLed(8,0,8);

				// Wait for 1 second
				if ((unsigned long)(millis() - startTime) > delayThreshold) threadState++;
			}
			else if (threadState == 2){
				_lowPressureValveConfiguration = 0;  // We can now garuntee that we are in the positive configuration

				HAL::set_C0_1_RgbLed_ReadySignal();
				threadState = 0;
				return 1; // Tell OSBos that the task is now finished
			}

			return returnCode;
		}
	}

	namespace tk_SetHighPressure_Positive {
		Thread Task = NewTerminalTask(Tick, 500);

		int8_t Tick(){
			static uint8_t threadState = 0;
			static unsigned long startTime = 0;
			static unsigned long delayThreshold = 2000;

			int8_t returnCode = 0;

			if (threadState == 0){
				startTime = millis();

				HAL::set_C0_1_RgbLed(8,0,8);

				_highPressureValveConfiguration = -1; // The state is uncertain once this task begins

				threadState++;
			}
			else if (threadState == 1){
				HAL::set_C0_1_RgbLed(8,0,8);

				// Wait for 1 second
				if ((unsigned long)(millis() - startTime) > delayThreshold) threadState++;
			}
			else if (threadState == 2){
				_highPressureValveConfiguration = 1;  // We can now garuntee that we are in the positive configuration

				HAL::set_C0_1_RgbLed_ReadySignal();
				threadState = 0;
				return 1; // Tell OSBos that the task is now finished
			}

			return returnCode;
		}
	}

	namespace tk_SetHighPressure_Negative {
		Thread Task = NewTerminalTask(Tick, 500);

		int8_t Tick(){
			static uint8_t threadState = 0;
			static unsigned long startTime = 0;
			static unsigned long delayThreshold = 2000;

			int8_t returnCode = 0;

			if (threadState == 0){
				startTime = millis();

				HAL::set_C0_1_RgbLed(8,0,8);

				_highPressureValveConfiguration = -1; // The state is uncertain once this task begins

				threadState++;
			}
			else if (threadState == 1){
				HAL::set_C0_1_RgbLed(8,0,8);

				// Wait for 1 second
				if ((unsigned long)(millis() - startTime) > delayThreshold) threadState++;
			}
			else if (threadState == 2){
				_highPressureValveConfiguration = 0;  // We can now garuntee that we are in the negative configuration

				HAL::set_C0_1_RgbLed_ReadySignal();
				threadState = 0;
				return 1; // Tell OSBos that the task is now finished
			}

			return returnCode;
		}
	}
}