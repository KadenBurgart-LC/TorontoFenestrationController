/* HAL.h

This file should act as the hardware abstraction layer for the application.
The application should NOT be accessing hardware devices directly, it should
only interface with hardware through this hardware abstraction layer.

If you need to break out complicated drivers into their own files, include them
through this file, and name them with the prefix hal_NameOfDriver.h

When including other files, please explain what they are and what they do.

*/

#ifndef HAL_H
#define HAL_H          

#include <stdint.h>
#include <Stream.h>
#include <Arduino.h>
#include <TimeLib.h>   // Used for working with time_t objects, mostly for the RTC         https://github.com/PaulStoffregen/Time

namespace HAL {
	enum class DigitalOutput {
		STRUCTURAL_BLOWER_POWER,
		LEAKAGE_BLOWER_POWER,
		WATER_PUMP_POWER,
		YELLOW_LED
	};

	enum class AnalogInput {
		PRESSURE_WINDOW_LOW,
		PRESSURE_WINDOW_MED,
		PRESSURE_WINDOW_HIGH,
		PRESSURE_LFE_DIFFERENTIAL,
		PRESSURE_LFE_ABSOLUTE,
		TEMP_LFE,
		TEMP_AMB,
		HUMIDITY_AMB,
		DISPLACEMENT_1,
		DISPLACEMENT_2,
		VALVE_C7_8, // leakage system cavity vent (main control valve) feedback signal
		VALVE_C7_1, // leakage system positive blower vent valve feedback signal
		VALVE_C7_2  // leakage system negative blower vent valve feedback signal
	};

	enum class AnalogOutput {
		VALVE_C7_8, // leakage system cavity vent (main control valve) feedback signal
		VALVE_C7_1, // leakage system positive blower vent valve feedback signal
		VALVE_C7_2  // leakage system negative blower vent valve feedback signal
	};

	enum class AnalogInputType {
		P1_08ADL_1,	// 8-CH analog current input module (4-20 mA)
		P1_08ADL_2, // 8-CH analog voltage input module (0-10 V)
		Simulation  // Don't read a real value, just regurgitate whatever is in LastRawUnitValue and LastRawSignalValue
	};

	struct AnalogInputDefinition {
		AnalogInputType InputType;

		const char* RawUnits = "";    // the raw input units (e.g. ADC counts or mA or V)
		const char* SignalUnits = ""; // the actual physical units of the output (e.g. mm or Pa or %)
		double SignalUnitGain = 1;    // the gain to convert from raw units to signal units (signal units = raw units * gain + offset)
		double SignalUnitOffset = 0;  // the offset to convert from raw units to signal units (signal units = raw units * gain + offset)

		int8_t P1_Slot = -1;
		int8_t P1_Channel = -1;

		double LastRawUnitValue = 0;	// The last value read
		double LastSignalUnitValue = 0; // The last value read
	};

	enum class AnalogOutputType {
		P1_08DAL_1  // 8-CH analog current output module (4-20 mA)
	};

	struct AnalogOutputDefinition {
		AnalogOutputType OutputType;

		const char* RawUnits = "";    // the raw output units (e.g. DAC counts or mA or V)
		const char* SignalUnits = ""; // the actual physical units of the output (e.g. mm or Pa or %)
		double SignalUnitGain = 1;	  // the gain to convert from raw units to signal units (signal units = raw units * gain + offset)
		double SignalUnitOffset = 0;  // the offset to convert from raw units to signal units (signal units = raw units * gain + offset)

		int8_t P1_Slot = -1;
		int8_t P1_Channel = -1;

		double LastRawUnitValue = 0;	// The last value read
		double LastSignalUnitValue = 0; // The last value read
	};

	/* Set the state of a digital output device
	   Returns...
	     0 - Output successfully turned OFF
	     1 - Output successfully turned ON
	     2 - Selected output not implemented - no action taken
	 */
	uint8_t setDigitalOutput(DigitalOutput o, bool state);
	bool getDigitalOutputState(DigitalOutput o);

	double getAnalogInput_RawUnits(AnalogInputDefinition& input, bool *error = nullptr);
	double getAnalogInput_RawUnits(AnalogInput i, bool *error = nullptr);
	double getAnalogInput_SignalUnits(AnalogInput i, bool *error = nullptr);

	bool setAnalogOutput_RawUnits(AnalogOutput o, double value); // returns false if there's an error
	bool setAnalogOutput_SignalUnits(AnalogOutput o, double value); // returns false if there's an error

	double getAnalogOutput_RawUnits_Last(AnalogOutputDefinition &output);
	double getAnalogOutput_RawUnits_Last(AnalogOutput o, bool *error = nullptr);
	double getAnalogOutput_SignalUnits_Last(AnalogOutputDefinition &output);
	double getAnalogOutput_SignalUnits_Last(AnalogOutput o, bool *error = nullptr);

	void init_CPU();
	void init_Serial();
	void init_P1Slots();

	// Set the colour of the RGB LED on the controller
	// WARNING: The LED is very bright. You probably don't want to turn it up higher than 10/255.
	void set_C0_1_RgbLed(uint8_t R, uint8_t G, uint8_t B);

	// Set the RGB LED to our "Ready" colour
	inline void set_C0_1_RgbLed_ReadySignal(){
		set_C0_1_RgbLed(1, 4, 0);
	}

	/* Print out all entries in a particular directory on the SD card.
       This function takes in any stream, so we can use the Serial interface or the web client. */
	void SD_PrintDirectory(Stream& printer, const char* dir);

	/* Print out the contents of a particular file on the SD card.
       This function takes in any stream, so we can use the Serial interface or the web client. 
       Returns true if the file was found and printed. */
	bool SD_PrintFileContents(Stream& printer, const char* filePath); 

	/* Take an input stream and write everything from that stream into a file on the SD card
	   until inputStream.available() isn't true anymore. 
	   If the file already exists, it will be deleted and replaced with the new data from the
	   input stream. 
	   Returns true if the file was written successfully. */
	bool SD_WriteFileFromStream(Stream& inputStream, const char* filePath);

	/* Take a C string and write the contents to a file.
	   If the file already exists, it will be deleted and replaced with the new data.
	   Retruns true if the file was written successfully. */
	bool SD_WriteFile(const char* dataToWrite, const char* filePath);

	/* Delete a file at the given path. 
	   Returns true if the file was found and deleted. */
	bool SD_DeleteFile(const char* filePath);

	/* Check if a directory exists. Make it if it doesn't */
	void SD_EnsureDirExists(const char* dir);

	/* Add to a file on the SD card. Mostly for logging. 
	   Returns true if we don't notice a problem (fail to open the file). */
	bool SD_AppendFile(const char* dataToAppend, const char* filePath);

	/* A function to set the date and time on the RTC that DOESN'T SUCK */
	void RTC_SetDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);

	/* Get the current time from the RTC, and return a computer-friendly struct with the date+time */
	//tmElements_t RTC_GetDateTime_Elements();

	/* A function to recall the current date and time from the Real-Time Clock */
	const char* RTC_GetDateTime();

	/* A function to recall the number of seconds that have passed today */
	uint16_t RTC_GetSecondsToday();

	/* A function to recall the current date/time in terms of seconds since Jan 1 1970 (unix timestamp) */
	time_t RTC_GetEpoch();

	/* Get the date in YYYY-MM-DD format, ignoring time. */
	const char* RTC_GetDate();

	const char* RTC_GetDate_Safe();
}

#endif