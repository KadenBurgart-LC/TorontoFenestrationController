#include "HAL.h"

#include <map>		   // standard c++ library
#include <array>       // standard c++ library
#include <Arduino.h>   // standard Arduino library
#include <P1AM.h>      // The public library for the AutomationDirect controller we are using      https://github.com/facts-engineering/P1AM
#include <Adafruit_NeoPixel.h> // used for the RGB LED on the P1AM-200    https://github.com/adafruit/Adafruit_NeoPixel
#include <SD.h>		   // standard Arduino library
#include <StreamUtils.h> // Used to turn a string into a stream for SD card file writing utilities    https://github.com/bblanchon/ArduinoStreamUtils

// Private members
namespace {
	// Keep an internal memory map of what values are assigned to each digital output
	std::map<HAL::DigitalOutput, bool> DO_States = {
		{HAL::DigitalOutput::STRUCTURAL_BLOWER_POWER, false},
		{HAL::DigitalOutput::LEAKAGE_BLOWER_POWER, false},
		{HAL::DigitalOutput::WATER_PUMP_POWER, false},
		{HAL::DigitalOutput::YELLOW_LED, false}
	};

	// Keep an internal memory map of what the slot number and channel number is for each 
	// AutomationDirect output module-style output.
	// [output, [slot number, channel number]]
	std::map<HAL::DigitalOutput, std::array<uint8_t, 2>> P1_DO_Channels = {
		{HAL::DigitalOutput::STRUCTURAL_BLOWER_POWER, {1, 1}},
		{HAL::DigitalOutput::LEAKAGE_BLOWER_POWER, {1, 2}},
		{HAL::DigitalOutput::WATER_PUMP_POWER, {1, 3}}
	};

	// Keep an internal memory map of what the pin number is for each Arduino style output
	std::map<HAL::DigitalOutput, uint8_t> Arduino_pins = {
		{HAL::DigitalOutput::YELLOW_LED, LED_BUILTIN}
	};

	// The RGB LED setup line from the P1AM-200 documentation
	Adafruit_NeoPixel C0_1_RgbLed(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

	uint8_t setDigitalOutputUtil(HAL::DigitalOutput o, bool state){
		DO_States[o] = state;
		return (uint8_t)state;
	}

	uint8_t setP1DigitalOutput(HAL::DigitalOutput o, bool state){
		P1.writeDiscrete(state, P1_DO_Channels[o][0], P1_DO_Channels[o][1]);

		return setDigitalOutputUtil(o, state);
	}

	uint8_t setArdPinDigitalOutput(HAL::DigitalOutput o, bool state){
		digitalWrite(Arduino_pins[o], state);

		return setDigitalOutputUtil(o, state);
	}
}

// Public members
namespace HAL {

	uint8_t setDigitalOutput(DigitalOutput o, bool state){
		uint8_t returnCode = 2; // Return code 2 means the selected output was not implemented and no action was taken

		if(o == DigitalOutput::STRUCTURAL_BLOWER_POWER) returnCode = setP1DigitalOutput(o, state);
		if(o == DigitalOutput::YELLOW_LED) returnCode = setArdPinDigitalOutput(o, state);

		return returnCode;
	}

	bool getDigitalOutputState(DigitalOutput o){
		return DO_States[o];
	}

	float getAnalogInput(AnalogInputs i){

	}

	void init_CPU(){
		while (!P1.init());

		// Start the driver for the RGB LED on the CPU
		C0_1_RgbLed.begin();

		/* Turn the RGB LED green
		   Note: The LED is very bright. No need to blind yourself. Turn it up to 255 if you are really mad at the Toronto people. */
		set_C0_1_RgbLed(1, 4, 0);

		/* Initialize the SD card module within the CPU.
		   The documentation for the P1AM-200 says that the chip select pin is stored in SDCARD_SS_PIN */	
		if(!SD.begin(SDCARD_SS_PIN)){
			Serial.println("Failed to initialize the SD card, which hosts the web app and logs data. Things to check...");
			Serial.println("  1. is the SD card inserted?");
			Serial.println("  2. is the SD card broken or full?");
			Serial.println("  3. is the SD card formatted with a FAT32 partition?");
		}
		else {
			Serial.println("SD card initialized.");
		}
	}

	void init_Serial(){
		Serial.begin(115200);
		while (!Serial);
	}

	void init_P1Slots(){
		const char P1_08ADL_2_CONFIG[] = { 0x40, 0x07 };
		P1.configureModule(P1_08ADL_2_CONFIG, 3); //sends the config data to the analog voltage input module in slot 3
	}

	// Set the colour of the RGB LED on the controller
	// WARNING: The LED is very bright. You probably don't want to turn it up higher than 10/255.
	void set_C0_1_RgbLed(uint8_t R, uint8_t G, uint8_t B){
		C0_1_RgbLed.setPixelColor(0, R, G, B);
		C0_1_RgbLed.show();
	}

	/* Print out all entries in a particular directory on the SD card.
	   This function takes in any stream, so we can use the Serial interface or the web client. */
	void SD_PrintDirectory(Stream& printer, char* dir){
		File dirf = SD.open(dir);

		printer.print("Listing the contents of directory \"");
		printer.print(dir);
		printer.print("\"\n");

		while(true){
			File entry = dirf.openNextFile();
			if(!entry) break;

			printer.print("\t");
			printer.print(entry.name());

			if(entry.isDirectory()) printer.println("/");
			else {
				printer.print("\t\t");
				printer.println(entry.size(), DEC);
			}
			entry.close();
		}
	}

	/* Print out the contents of a particular file on the SD card.
	   This function takes in any stream, so we can use the Serial interface or the web client. 
	   Returns true if the file was found and printed. */
	bool SD_PrintFileContents(Stream& printer, char* filePath){
		bool success = false;
		File sdFile = SD.open(filePath);

		if(sdFile){
			while(sdFile.available()) printer.write(sdFile.read());
			sdFile.close();
			success = true;
		}
		else {
			printer.print("Failed to open file \"");
			printer.print(filePath);
			printer.print("\"\nAre you sure this file exists?");
		}

		return success;
	}

	/* Take an input stream and write everything from that stream into a file on the SD card
	   until inputStream.available() isn't true anymore. 
	   If the file already exists, it will be deleted and replaced with the new data from the
	   input stream. 
	   Returns true if the file was written successfully. */
	bool SD_WriteFileFromStream(Stream& inputStream, char* filePath){
		bool success = false;

		// Delete the file if it already exists
		if(SD.exists(filePath)) SD.remove(filePath);

		File sdFile = SD.open(filePath, FILE_WRITE);
		while(inputStream.available()) sdFile.write(inputStream.read());
		sdFile.close();

		if(SD.exists(filePath)) success = true;

		return success;
	}

	/* Take a C string and write the contents to a file.
	   If the file already exists, it will be deleted and replaced with the new data.
   	   Retruns true if the file was written successfully. */
	bool SD_WriteFile(char* dataToWrite, char* filePath){
		StringStream fakeStream;

		fakeStream.print(dataToWrite);

		return SD_WriteFileFromStream(fakeStream, filePath);
	}

	/* Delete a file at the given path. 
	   Returns true if the file was found and deleted. */
	bool SD_DeleteFile(char* filePath){
		bool success = false;
		bool fileFound = false;

		if(SD.exists(filePath)){
			fileFound = true;
			SD.remove(filePath);
		}
		if(fileFound && !SD.exists(filePath)) success = true;

		return success;
	}

	/* Add to a file on the SD card. Mostly for logging. 
	   Returns true if we don't fail to open the file. */
	bool SD_AppendFile(char* dataToAppend, char* filePath){
		bool success = false;
		File sdFile = SD.open(filePath, FILE_WRITE);

		if(sdFile){
			sdFile.print(dataToAppend);
			sdFile.close();

			if(SD.exists(filePath)) success = true;
		}

		return success;
	}
}