#include "HAL.h"

#include <map>		   // standard c++ library
#include <array>       // standard c++ library
#include <string.h>    // standard c++ library
#include <Arduino.h>   // standard Arduino library
#include <SD.h>		   // standard Arduino library
#include <P1AM.h>      // The public library for the AutomationDirect P1AM-200 controller we are using      https://github.com/facts-engineering/P1AM
#include <Adafruit_NeoPixel.h> // used for the RGB LED on the P1AM-200    https://github.com/adafruit/Adafruit_NeoPixel
#include <StreamUtils.h> // Used to turn a string into a stream for SD card file writing utilities    https://github.com/bblanchon/ArduinoStreamUtils
#include <PCF8563.h>   // Used for the RTC on the P1AM-200; I hate this library        https://github.com/facts-engineering/PCF8563_RTC
#include <TimeLib.h>   // Used for working with time_t objects, mostly for the RTC         https://github.com/PaulStoffregen/Time

// Private members
namespace {
	// Keep an internal memory map of what values are assigned to each digital output
	std::map<HAL::DigitalOutput, bool> DO_States = {
		{HAL::DigitalOutput::STRUCTURAL_BLOWER_POWER, false},
		{HAL::DigitalOutput::LEAKAGE_BLOWER_POWER, false},
		{HAL::DigitalOutput::WATER_PUMP_POWER, false},
		{HAL::DigitalOutput::YELLOW_LED, false}
	};

	/* Keep an internal memory map of what the slot number and channel number is for each 
	   AutomationDirect output module-style output.
	   [output, [slot number, channel number]] */
	std::map<HAL::DigitalOutput, std::array<uint8_t, 2>> P1_DO_Channels = {
		{HAL::DigitalOutput::STRUCTURAL_BLOWER_POWER, {1, 1}},
		{HAL::DigitalOutput::LEAKAGE_BLOWER_POWER, {1, 2}},
		{HAL::DigitalOutput::WATER_PUMP_POWER, {1, 3}}
	};

	// Keep an internal memory map of what the pin number is for each Arduino style output
	std::map<HAL::DigitalOutput, uint8_t> Arduino_pins = {
		{HAL::DigitalOutput::YELLOW_LED, LED_BUILTIN}
	};

	// Keep an internal memory map of how to read each analog input
	std::map<HAL::AnalogInput, HAL::AnalogSignalDefinition> AnalogSignals = {
		{HAL::AnalogInput::PRESSURE_WINDOW_LOW, ([]() {
			HAL::AnalogSignalDefinition def;

			def.MeasurementType = HAL::AnalogIntermediateMeasurementType::CURRENT;
			def.IntermediateMeasurementType_ConversionDenominator = 409.55;	// This is the denominator for a P1 analog current channel

			def.SignalUnits = "Pa";
			def.SignalUnitGain = 1;
			def.SignalUnitOffset = 0;

			def.P1_Slot = 2;
			def.P1_Channel = 1;

			return def;
		})()},
		{HAL::AnalogInput::PRESSURE_WINDOW_MED, ([]() {
			HAL::AnalogSignalDefinition def;

			def.MeasurementType = HAL::AnalogIntermediateMeasurementType::CURRENT;
			def.IntermediateMeasurementType_ConversionDenominator = 409.55;	// This is the denominator for a P1 analog current channel

			def.SignalUnits = "Pa";
			def.SignalUnitGain = 1;
			def.SignalUnitOffset = 0;

			def.P1_Slot = 2;
			def.P1_Channel = 1;

			return def;
		})()},
		{HAL::AnalogInput::PRESSURE_WINDOW_HIGH, ([]() {
			HAL::AnalogSignalDefinition def;

			def.MeasurementType = HAL::AnalogIntermediateMeasurementType::CURRENT;
			def.IntermediateMeasurementType_ConversionDenominator = 409.55;	// This is the denominator for a P1 analog current channel

			def.SignalUnits = "Pa";
			def.SignalUnitGain = 1;
			def.SignalUnitOffset = 0;

			def.P1_Slot = 2;
			def.P1_Channel = 1;

			return def;
		})()},
		{HAL::AnalogInput::PRESSURE_LFE_DIFFERENTIAL, ([]() {
			HAL::AnalogSignalDefinition def;

			def.MeasurementType = HAL::AnalogIntermediateMeasurementType::CURRENT;
			def.IntermediateMeasurementType_ConversionDenominator = 409.55;	// This is the denominator for a P1 analog current channel

			def.SignalUnits = "Pa";
			def.SignalUnitGain = 1;
			def.SignalUnitOffset = 0;

			def.P1_Slot = 2;
			def.P1_Channel = 1;

			return def;
		})()},
		{HAL::AnalogInput::DISPLACEMENT_1, ([]() {
			HAL::AnalogSignalDefinition def;

			def.MeasurementType = HAL::AnalogIntermediateMeasurementType::CURRENT;
			def.IntermediateMeasurementType_ConversionDenominator = 409.55;	// This is the denominator for a P1 analog current channel

			def.SignalUnits = "Pa";
			def.SignalUnitGain = 1;
			def.SignalUnitOffset = 0;

			def.P1_Slot = 2;
			def.P1_Channel = 1;

			return def;
		})()},
		{HAL::AnalogInput::DISPLACEMENT_2, ([]() {
			HAL::AnalogSignalDefinition def;

			def.MeasurementType = HAL::AnalogIntermediateMeasurementType::CURRENT;
			def.IntermediateMeasurementType_ConversionDenominator = 409.55;	// This is the denominator for a P1 analog current channel

			def.SignalUnits = "Pa";
			def.SignalUnitGain = 1;
			def.SignalUnitOffset = 0;

			def.P1_Slot = 2;
			def.P1_Channel = 1;

			return def;
		})()}
	};

	// The RGB LED setup line from the P1AM-200 documentation
	Adafruit_NeoPixel C0_1_RgbLed(1, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

	// uint8_t setDigitalOutputUtil(HAL::DigitalOutput o, bool state){
	// 	DO_States[o] = state;
	// 	return (uint8_t)state;
	// }

	// uint8_t setP1DigitalOutput(HAL::DigitalOutput o, bool state){
	// 	P1.writeDiscrete(state, P1_DO_Channels[o][0], P1_DO_Channels[o][1]);

	// 	return setDigitalOutputUtil(o, state);
	// }

	// uint8_t setArdPinDigitalOutput(HAL::DigitalOutput o, bool state){
	// 	digitalWrite(Arduino_pins[o], state);

	// 	return setDigitalOutputUtil(o, state);
	// }
}

// Public members
namespace HAL {

	uint8_t setDigitalOutput(DigitalOutput o, bool state){
		uint8_t returnCode = 2; // Return code 2 means the selected output was not implemented and no action was taken

		auto ix1 = P1_DO_Channels.find(o);
		auto ix2 = Arduino_pins.find(o);

		if (ix1 != P1_DO_Channels.end()) {
			uint8_t slot = (ix1->second)[0];
			uint8_t channel = (ix1->second)[1];

			P1.writeDiscrete(state, slot, channel);
			DO_States[o] = state;

			return (uint8_t)state;
		}
		else if (ix2 != Arduino_pins.end()) {
			uint8_t pin = ix2->second;

			digitalWrite(Arduino_pins[o], state);
			DO_States[o] = state;

			return (uint8_t)state;
		}

		//if(o == DigitalOutput::STRUCTURAL_BLOWER_POWER) returnCode = setP1DigitalOutput(o, state);
		//if(o == DigitalOutput::YELLOW_LED) returnCode = setArdPinDigitalOutput(o, state);

		return returnCode;
	}

	bool getDigitalOutputState(DigitalOutput o){
		return DO_States[o];
	}

	float getAnalogInputFloat(AnalogInput i){
		auto ix = AnalogSignals.find(i);

		if (ix != AnalogSignals.end()){
			AnalogSignalDefinition& sig = ix->second;

			if (sig.P1_Slot > 0 && sig.P1_Channel > 0){
				sig.LastIntermediateValue =  ((float)P1.readAnalog(sig.P1_Slot, sig.P1_Channel)) / sig.IntermediateMeasurementType_ConversionDenominator;
				sig.LastSignalUnitValue = sig.LastIntermediateValue * sig.SignalUnitGain + sig.SignalUnitOffset;

				return sig.LastSignalUnitValue;
			}
			else return 0;
		}
		else return 0;
	}

	void init_CPU(){
		while (!P1.init());

		// Start the driver for the RGB LED on the CPU
		C0_1_RgbLed.begin();

		/* Turn the RGB LED green
		   Note: The LED is very bright. No need to blind yourself. Turn it up to 255 if you are really mad at the Toronto people. */
		set_C0_1_RgbLed_ReadySignal();

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

	/* Set the colour of the RGB LED on the controller
	   WARNING: The LED is very bright. You probably don't want to turn it up higher than 10/255. */
	void set_C0_1_RgbLed(uint8_t R, uint8_t G, uint8_t B){
		C0_1_RgbLed.setPixelColor(0, R, G, B);
		C0_1_RgbLed.show();
	}

	/* Print out all entries in a particular directory on the SD card.
	   This function takes in any stream, so we can use the Serial interface or the web client. */
	void SD_PrintDirectory(Stream& printer, const char* dir){
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
	bool SD_PrintFileContents(Stream& printer, const char* filePath){
		bool success = false;
		File sdFile = SD.open(filePath);
		uint8_t buffer[64];

		if(sdFile){
			while(sdFile.available()){
				int bytesRead = sdFile.read(buffer, sizeof(buffer));
				printer.write(buffer, bytesRead);
			}
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
	bool SD_WriteFileFromStream(Stream& inputStream, const char* filePath){
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
	bool SD_WriteFile(const char* dataToWrite, const char* filePath){
		StringStream fakeStream;

		fakeStream.print(dataToWrite);

		return SD_WriteFileFromStream(fakeStream, filePath);
	}

	/* Delete a file at the given path. 
	   Returns true if the file was found and deleted. */
	bool SD_DeleteFile(const char* filePath){
		bool success = false;
		bool fileFound = false;

		if(SD.exists(filePath)){
			fileFound = true;
			SD.remove(filePath);
		}
		if(fileFound && !SD.exists(filePath)) success = true;

		return success;
	}

	/* Check if a directory exists. Make it if it doesn't */
	bool SD_EnsureDirExists(const char* dir){
		if(!SD.exists(dir)) SD.mkdir(dir);
	}

	/* Add to a file on the SD card. Mostly for logging. 
	   Returns true if we don't fail to open the file. */
	bool SD_AppendFile(const char* dataToAppend, const char* filePath){
		bool success = false;
		File sdFile = SD.open(filePath, FILE_WRITE);

		if(sdFile){
			sdFile.print(dataToAppend);
			sdFile.close();

			if(SD.exists(filePath)) success = true;
		}

		return success;
	}

	/* A function to set the date and time on the RTC that DOESN'T SUCK FERMENTED ASSHOLE */
	void RTC_SetDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second){
		tmElements_t te;

		te.Year = year - 1970; // Yeah it's weird it's just how the library works.
		te.Month = month;
		te.Day = day;
		te.Hour = hour;
		te.Minute = minute;
		te.Second = second;

		time_t t = makeTime(te);

		PCF8563_RTC.setEpoch(t);
	}

	//tmElements_t RTC_GetDateTime_Elements(){ return PCF8563_RTC.getEpoch(); }

	/* A function to recall the current date and time from the Real-Time Clock */
	String RTC_GetDateTime(){
		String dateTimeStr = "";
		time_t t = PCF8563_RTC.getEpoch();

		dateTimeStr += year(t);
		dateTimeStr += "-";
		dateTimeStr += (month(t) < 10) ? "0" : "";
		dateTimeStr += month(t);
		dateTimeStr += "-";
		dateTimeStr += (day(t) < 10) ? "0" : "";
		dateTimeStr += day(t);
		dateTimeStr += " ";
		dateTimeStr += (hour(t) < 10) ? "0" : "";
		dateTimeStr += hour(t);
		dateTimeStr += ":";
		dateTimeStr += (minute(t) < 10) ? "0" : "";
		dateTimeStr += minute(t);
		dateTimeStr += ":";
		dateTimeStr += (second(t) < 10) ? "0" : "";
		dateTimeStr += second(t);

		return dateTimeStr;
	}

	/* A function to recall the current date/time in terms of seconds since Jan 1 1970 (unix timestamp) */
	time_t RTC_GetEpoch(){
		return PCF8563_RTC.getEpoch();
	}

	/* Get the date in YYYY-MM-DD format, ignoring time. */
	String RTC_GetDate(){
		String dateStr = "";
		time_t t = PCF8563_RTC.getEpoch();

		dateStr += year(t);
		dateStr += "-";
		dateStr += (month(t) < 10) ? "0" : "";
		dateStr += month(t);
		dateStr += "-";
		dateStr += (day(t) < 10) ? "0" : "";
		dateStr += day(t);

		return dateStr;
	}

}