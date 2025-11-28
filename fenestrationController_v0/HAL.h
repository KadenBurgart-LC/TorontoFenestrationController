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

namespace HAL {
	enum class DigitalOutput {
		STRUCTURAL_BLOWER_POWER = 1,
		LEAKAGE_BLOWER_POWER = 2,
		WATER_PUMP_POWER = 3,
		YELLOW_LED = 4,
		POSITIVE_PRESSURE
	};

	enum class AnalogInputs {
		PRESSURE_WINDOW_LOW = 0,
		PRESSURE_WINDOW_MED = 1,
		PRESSURE_WINDOW_HIGH = 2,
		PRESSURE_LFE_DIFFERENTIAL = 3,
		PRESSURE_LFE_ABS = 4,
		DISPLACEMENT_1 = 5,
		DISPLACEMENT_2 = 6,
		VALVE_C7_8 = 7, // leakage system cavity vent (main control valve) feedback signal
		VALVE_C7_1 = 8, // leakage system positive blower vent valve feedback signal
		VALVE_C7_2 = 9  // leakage system negative blower vent valve feedback signal
	};

	/* Set the state of a digital output device
	   Returns...
	     0 - Output successfully turned OFF
	     1 - Output successfully turned ON
	     2 - Selected output not implemented - no action taken
	 */
	uint8_t setDigitalOutput(DigitalOutput o, bool state);

	bool getDigitalOutputState(DigitalOutput o, bool state);

	float getAnalogInput(AnalogInputs i);

	void init_CPU();
	void init_Serial();
	void init_P1Slots();

	// Set the colour of the RGB LED on the controller
	// WARNING: The LED is very bright. You probably don't want to turn it up higher than 10/255.
	void set_C0_1_RgbLed(uint8_t R, uint8_t G, uint8_t B);

	/* Print out all entries in a particular directory on the SD card.
       This function takes in any stream, so we can use the Serial interface or the web client. */
	void SD_PrintDirectory(Stream& printer, char* dir);

	/* Print out the contents of a particular file on the SD card.
       This function takes in any stream, so we can use the Serial interface or the web client. 
       Returns true if the file was found and printed. */
	bool SD_PrintFileContents(Stream& printer, char* filePath); 

	/* Take an input stream and write everything from that stream into a file on the SD card
	   until inputStream.available() isn't true anymore. 
	   If the file already exists, it will be deleted and replaced with the new data from the
	   input stream. 
	   Returns true if the file was written successfully. */
	bool SD_WriteFileFromStream(Stream& inputStream, char* filePath);

	/* Take a C string and write the contents to a file.
	   If the file already exists, it will be deleted and replaced with the new data.
	   Retruns true if the file was written successfully. */
	bool SD_WriteFile(char* dataToWrite, char* filePath);

	/* Delete a file at the given path. 
	   Returns true if the file was found and deleted. */
	bool SD_DeleteFile(char* filePath);

	/* Add to a file on the SD card. Mostly for logging. 
	   Returns true if we don't notice a problem (fail to open the file). */
	bool SD_AppendFile(char* dataToAppend, char* filePath);
}

#endif