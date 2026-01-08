#include "th_DataLogger.h"

#include <Arduino.h>   // standard Arduino library
#include <SD.h>		   // standard Arduino library
#include <TimeLib.h>   // Used for working with time_t objects, mostly for the RTC         https://github.com/PaulStoffregen/Time
#include "HAL.h"       // Our hardware abstraction layer - used for SD card and RTC functions
#include <cstring>     // Standard c++
#include <stdio.h>     // Standard c++
#include <string>      // Standard c++
#include <avr/dtostrf.h> // Needed for dtostrf on SAMD/ARM cores
#include <math.h> 	   // Standard c++, used for rounding
#include "MechanicalSystem.h"

namespace {

	#define LOG_LINE_DATA_STR_LENGTH 400 // 200 chars for the main portion of the log and another 200 chars for the note
	#define LOG_BUFFER_LENGTH 20         // We store the last 20 log rows in RAM
	#define LOG_FILE_MAX_PATH_LENGTH 200 // The log file path on the SD card can be up to 200 chars long

	struct LogEntry {
		uint64_t fullTimestamp;				// Seconds since 1970-1-1 * 10 + number of entries made this second
		char dataLine[LOG_LINE_DATA_STR_LENGTH]; // All characters (including timestamp) EXCEPT the user note
	};

	LogEntry _logBuffer[LOG_BUFFER_LENGTH];
	uint8_t _bufferIX = 0;

	uint64_t _lastTimestamp = 0;

	const char* _getCurrentLogFilePath(){
		static char logFilePath[LOG_FILE_MAX_PATH_LENGTH]; 

		snprintf(logFilePath, sizeof(logFilePath), "logs/%s.csv", HAL::RTC_GetDate_Safe());

		return logFilePath;
	}

	void _uint64ToString(uint64_t n, char* buf) {
	    char temp[21]; // Max digits for uint64_t + null
	    int i = 0;
	    if (n == 0) {
	        temp[i++] = '0';
	    } else {
	        while (n > 0) {
	            temp[i++] = (n % 10) + '0';
	            n /= 10;
	        }
	    }
	    // Reverse the string into the target buffer
	    for (int j = 0; j < i; j++) {
	        buf[j] = temp[i - 1 - j];
	    }
	    buf[i] = '\0';
	}

	uint8_t _logDataRow(const char* adtNote = "_"){
		LogEntry &entry = _logBuffer[_bufferIX++];

		if (_bufferIX >= LOG_BUFFER_LENGTH) _bufferIX = 0;

		entry.fullTimestamp = (uint64_t)HAL::RTC_GetEpoch() * 10;

		if (entry.fullTimestamp <= _lastTimestamp){
			if(_lastTimestamp % 10 < 9) entry.fullTimestamp = _lastTimestamp + 1;
			else return -1; // can't write more than 10 log entries per second
		}
		_lastTimestamp = entry.fullTimestamp;

		// Convert floats to character buffers.
		// Increase all buffer sizes by 2 MORE bytes to allow for stupid.
		char ts[15];
		char tp[11]; // targetPressure (Pa)  PG100 goes to 9600 Pa. PG200 goes to 14390.0 Pa (7 chars +1 +1 )
		char lowPres[8];  // lowPressureSensor (Pa) (one decimal place) (4 chars +1 for "-", +1 for "\0")
		char dis1[9]; // displacement1 (mm) (2 decimal places) (5 chars +1 for "-", +1 for "\0")
		char dis2[9]; // displacement2 (mm) (2 decimal places) (5 chars +1 for "-", +1 for "\0")
		char lfeD[11]; // LFE differential pressure (Pa) (two decimal places) (7 chars +1 for "-", +1 for "\0")
		char lfeA[9]; // LFE abs pressure (kPa) (two decimal places) (6 chars +1 for "\0")
		char lfeT[8]; // LFE air temperature (°C) (one decimal place) (4 chars +1 for "-", +1 for "\0")
		char ambT[8]; // Ambient air temperature (°C) (one decimal place) (4 chars +1 for "-", +1 for "\0")
		char ambH[7]; // Ambient humidity (%) (one decimal place) (4 chars +1 for "\0")
		_uint64ToString(entry.fullTimestamp, ts);
		dtostrf(MechanicalSystem::GetTargetPressure(), 0, 1, tp);
		dtostrf(HAL::getAnalogInput_SignalUnits(HAL::AnalogInput::PRESSURE_WINDOW_LOW), 0, 1, lowPres);
		dtostrf(HAL::getAnalogInput_SignalUnits(HAL::AnalogInput::DISPLACEMENT_1), 0, 2, dis1);
		dtostrf(HAL::getAnalogInput_SignalUnits(HAL::AnalogInput::DISPLACEMENT_2), 0, 2, dis2);
		dtostrf(HAL::getAnalogInput_SignalUnits(HAL::AnalogInput::PRESSURE_LFE_DIFFERENTIAL), 0, 2, lfeD);
		dtostrf(HAL::getAnalogInput_SignalUnits(HAL::AnalogInput::PRESSURE_LFE_ABSOLUTE), 0, 2, lfeA);
		dtostrf(HAL::getAnalogInput_SignalUnits(HAL::AnalogInput::TEMP_LFE), 0, 1, lfeT);
		dtostrf(HAL::getAnalogInput_SignalUnits(HAL::AnalogInput::TEMP_AMB), 0, 1, ambT);
		dtostrf(HAL::getAnalogInput_SignalUnits(HAL::AnalogInput::HUMIDITY_AMB), 0, 1, ambH);

		//Serial.println("just before snprintf");

		int n = snprintf(
				entry.dataLine,
				LOG_LINE_DATA_STR_LENGTH,
				"%s,%s,%s,%s,%i,%i,%s,%s,%s,%s,%s,%s,%s,%i,%i,%i,%i,%i,%s\n",
				ts, // (11 chars)
				HAL::RTC_GetDateTime(), // (19 chars)
				tp,  // targetPressure (Pa)  PG100 goes to 9600 Pa. PG200 goes to 14390 Pa (5 chars)
				lowPres, // lowPressureSensor (Pa) (one decimal place) (4 chars)
				(int)round(HAL::getAnalogInput_SignalUnits(HAL::AnalogInput::PRESSURE_WINDOW_MED)),  // medPressureSensor (Pa) (no decimal places) (5 chars)
				(int)round(HAL::getAnalogInput_SignalUnits(HAL::AnalogInput::PRESSURE_WINDOW_HIGH)), // highPressureSensor (Pa) (5 chars)
				dis1, // displacement1 (mm) (5 chars)
				dis2, // displacement2 (mm) (5 chars)
				lfeD, // LFE differential pressure (Pa) (two decimal places) (7 chars)
				lfeA, // LFE abs pressure (kPa) (two decimal places) (6 chars)
				lfeT, // LFE air temperature (°C) (one decimal place) (4 chars)
				ambT, // Ambient air temperature (°C) (one decimal place) (4 chars)
				ambH, // Ambient humidity (%) (one decimal place) (4 chars)
				MechanicalSystem::GetLowPressureValveConfiguration(), // LP_dir (0 for negative, 1 for positive, -1 for undefined) (2 chars)
				MechanicalSystem::GetHighPressureValveConfiguration(), // HP_dir (0 for negative, 1 for positive, -1 for undefined) (2 chars)
				(int)HAL::getDigitalOutputState(HAL::DigitalOutput::LEAKAGE_BLOWER_POWER), // LP_blower_on (0 or 1) (1 chars)
				(int)HAL::getDigitalOutputState(HAL::DigitalOutput::STRUCTURAL_BLOWER_POWER), // HP_blower_on (0 or 1) (1 chars)
				(int)HAL::getDigitalOutputState(HAL::DigitalOutput::WATER_PUMP_POWER), // water pump on (0 or 1) (1 chars)
				adtNote
			);

		//Serial.println(n);
		if(n <= 0 || n >= (int)sizeof(entry.dataLine)) return -1;

		const char* logFilePath = _getCurrentLogFilePath();
		
		HAL::SD_EnsureDirExists("logs");
		
		if(HAL::SD_AppendFile(entry.dataLine, logFilePath)) return 1; // success
		else return -1; // action failed
	}
}

namespace th_DataLogger {
	//void initialize(){} // I don't think we need this?

	int8_t tick(){
		if(logStandardDataRow() != 1) Serial.println("th_DataLogger: ERROR: Failed to write a log row.");

		return 1;
	}

	// Add a new "standard row" to the data log
	uint8_t logStandardDataRow(){
		return _logDataRow();
	}

	// Add some special note to the data log. We put this note in a seperate column at the end of the normal log data to make data processing easier for our poor technicians
	uint8_t writeToLog(const char* str){
		return _logDataRow(str);
	}

	// Return the last line which was logged
	const char* getLastLogLine(){
		uint8_t lastIX = (_bufferIX == 0) ? LOG_BUFFER_LENGTH - 1 : _bufferIX - 1;
		return _logBuffer[lastIX].dataLine;
	}

	// Return a string with all log rows that were generated AFTER this timestamp
	const char* getLogsSince(uint64_t timeStamp){
		static char logBufferDump[LOG_LINE_DATA_STR_LENGTH * LOG_BUFFER_LENGTH];
		logBufferDump[0] = '\0';

		char* writePtr = logBufferDump;
		int remainingSpace = sizeof(logBufferDump);
		for(int i=0; i<LOG_BUFFER_LENGTH; i++){
			uint8_t seekIx = (_bufferIX + i) % LOG_BUFFER_LENGTH;
			if(_logBuffer[seekIx].fullTimestamp > timeStamp){
				int written = snprintf(writePtr, remainingSpace, "%s", _logBuffer[seekIx].dataLine);
				if(written > 0 && written < remainingSpace){
					writePtr += written;
					remainingSpace -= written;
				} else break;
			}
		}
		return logBufferDump;
	}

	const char* getCurrentLogFilePath(){ return _getCurrentLogFilePath(); }
}