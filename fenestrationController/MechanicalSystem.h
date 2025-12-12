#ifndef MECHANICAL_SYSTEM_H
#define MECHANICAL_SYSTEM_H

#include <OSBos.h>      // The custom library that acts as our simple kernel https://github.com/actuvon/OSBos
#include "HAL.h"
#include "Arduino.h"

/* This file contains behaviors relating to the business logic of the mechanical system.
 * Tasks defined in this file live inside of their own namespaces labeled tk_NameOfTask.
 * Threads defined in this file live inside their own namespaces labeled th_NameOfThread.
 */

namespace MechanicalSystem {

	/* Get or Set the target pressure for the fenestration wall in Pa.
	   The Set function will return true if the command worked or false if it did not.
	   This is the value that the PID loop will work towards attaining. */
	bool SetTargetPressure(float pressure);
	float GetTargetPressure();

	int8_t GetLowPressureValveConfiguration();
	int8_t GetHighPressureValveConfiguration();

	namespace th_HighPressurePID_RUN { }
	namespace th_LowPressurePID_RUN { }

	namespace tk_StopAll {
		extern Thread Task;
		int8_t Tick();
	}

	namespace tk_SetLowPressure_Positive { 
		extern Thread Task;
		int8_t Tick();
	}
	namespace tk_SetLowPressure_Negative { 
		extern Thread Task;
		int8_t Tick();
	}
	namespace tk_SetHighPressure_Positive { }
	namespace tk_SetHighPressure_Negative { }

}

#endif