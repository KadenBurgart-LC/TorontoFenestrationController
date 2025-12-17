#include "th_DataLogger.h"

#include <Arduino.h>   // standard Arduino library
#include <SD.h>		   // standard Arduino library
#include <TimeLib.h>   // Used for working with time_t objects, mostly for the RTC         https://github.com/PaulStoffregen/Time
#include "HAL.h"       // Our hardware abstraction layer - used for SD card and RTC functions
#include <cstring>     // Standard c++
#include <stdio.h>     // Standard c++
#include <string>      // Standard c++
#include <avr/dtostrf.h> // Needed for dtostrf on SAMD/ARM cores

namespace {

	#define LOG_LINE_DATA_STR_LENGTH 200
	#define LOG_BUFFER_LENGTH 5

	struct LogEntry {
		uint64_t fullTimestamp;				// Seconds since 1970-1-1 * 10 + number of entries made this second
		char dataLine[LOG_LINE_DATA_STR_LENGTH]; // All characters (including timestamp) EXCEPT the user note
		String note;
	};

	LogEntry _logBuffer[LOG_BUFFER_LENGTH];
	uint8_t _bufferIX = 0;

	uint64_t _lastTimestamp = 0;

	String _getCurrentLogFilePath(){
		String logFilePath = "logs/";
		logFilePath += HAL::RTC_GetDate_Safe();
		logFilePath += ".csv";

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

	uint8_t _logDataRow(String adtNote = "_"){
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
		char lowPres[8];  // lowPressureSensor (Pa) (one decimal place) (4 chars +1 for "-", +1 for "\0")
		char dis1[9]; // displacement1 (mm) (2 decimal places) (5 chars +1 for "-", +1 for "\0")
		char dis2[9]; // displacement2 (mm) (2 decimal places) (5 chars +1 for "-", +1 for "\0")
		char lfeD[11]; // LFE differential pressure (Pa) (two decimal places) (7 chars +1 for "-", +1 for "\0")
		char lfeA[9]; // LFE abs pressure (kPa) (two decimal places) (6 chars +1 for "\0")
		char lfeT[8]; // LFE air temperature (°C) (one decimal place) (4 chars +1 for "-", +1 for "\0")
		char ambT[8]; // Ambient air temperature (°C) (one decimal place) (4 chars +1 for "-", +1 for "\0")
		char ambH[7]; // Ambient humidity (%) (one decimal place) (4 chars +1 for "\0")
		_uint64ToString(entry.fullTimestamp, ts);
		dtostrf(75.8, 0, 1, lowPres);
		dtostrf(99.04, 0, 2, dis1);
		dtostrf(99.27, 0, 2, dis2);
		dtostrf(1990.72, 0, 2, lfeD);
		dtostrf(102.88, 0, 2, lfeA);
		dtostrf(25.4, 0, 1, lfeT);
		dtostrf(21.2, 0, 1, ambT);
		dtostrf(38.1, 0, 1, ambH);

		//Serial.println("just before snprintf");

		int n = snprintf(
			entry.dataLine,
			LOG_LINE_DATA_STR_LENGTH,
			"%s,%s,%i,%s,%i,%i,%s,%s,%s,%s,%s,%s,%s,%i,%i,%i,%i,%i,",
			ts, // (11 chars)
			HAL::RTC_GetDateTime().c_str(), // (19 chars)
			14390,  // targetPressure (Pa)  PG100 goes to 9600 Pa. PG200 goes to 14390 Pa (5 chars)
			lowPres, // lowPressureSensor (Pa) (one decimal place) (4 chars)
			9500,  // medPressureSensor (Pa) (no decimal places) (5 chars)
			14000,  // highPressureSensor (Pa) (5 chars)
			dis1, // displacement1 (mm) (5 chars)
			dis2, // displacement2 (mm) (5 chars)
			lfeD, // LFE differential pressure (Pa) (two decimal places) (7 chars)
			lfeA, // LFE abs pressure (kPa) (two decimal places) (6 chars)
			lfeT, // LFE air temperature (°C) (one decimal place) (4 chars)
			ambT, // Ambient air temperature (°C) (one decimal place) (4 chars)
			ambH, // Ambient humidity (%) (one decimal place) (4 chars)
			0, // LP_dir (-1 for negative, 1 for positive, X for undefined) (2 chars)
			-1, // HP_dir (-1 for negative, 1 for positive, X for undefined) (2 chars)
			0, // LP_blower_on (0 or 1) (1 chars)
			1, // HP_blower_on (0 or 1) (1 chars)
			0 // water pump on (0 or 1) (1 chars)
			);
		entry.note = adtNote;

		//Serial.println(n);
		if(n <= 0 || n >= (int)sizeof(entry.dataLine)) return -1;

		String fullLine = String(entry.dataLine) + adtNote + "\n";
		String logFilePath = _getCurrentLogFilePath();
		
		HAL::SD_EnsureDirExists("logs");

		Serial.println(logFilePath);
		
		if(HAL::SD_AppendFile(fullLine.c_str(), logFilePath.c_str())) return 1; // success
		else return -1; // action failed
	}
}

namespace th_DataLogger {
	//void initialize(){} // I don't think we need this?

	int8_t tick(){
		if(logStandardDataRow() != 1) Serial.println("th_DataLogger: ERROR: Failed to write a log row.");

		return 1;
	}

	uint8_t logStandardDataRow(){
		return _logDataRow();
	}

	uint8_t writeToLog(String str){
		return _logDataRow(str);
	}

	String getLastLogLine(){
		uint8_t lastIX = (_bufferIX == 0) ? LOG_BUFFER_LENGTH - 1 : _bufferIX - 1;

		LogEntry lastEntry = _logBuffer[lastIX];

		String lastLine = String(lastEntry.dataLine) + lastEntry.note;

		return lastLine;
	}

	String getCurrentLogFilePath(){ return _getCurrentLogFilePath(); }
}