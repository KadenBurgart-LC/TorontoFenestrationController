#include "HAL.h"

#include <map>
#include <array>
#include <P1AM.h>      // The public library for the AutomationDirect controller we are using
#include <Arduino.h>     
#include <Adafruit_NeoPixel.h>

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

		// Turn the RGB LED green
		// Note: The LED is very bright. No need to blind yourself. Turn it up to 255 if you are really mad at the Toronto people.
		set_C0_1_RgbLed(1, 4, 0);
	}

	void init_Serial(){
		Serial.begin(115200);
		while (!Serial);
	}

	void init_P1Slots(){
		const char P1_08ADL_2_CONFIG[] = { 0x40, 0x07 };
		P1.configureModule(P1_08ADL_2_CONFIG, 3); //sends the config data to the analog voltage input module in slot 3
	}

	void set_C0_1_RgbLed(uint8_t R, uint8_t G, uint8_t B){
		C0_1_RgbLed.setPixelColor(0, R, G, B);
		C0_1_RgbLed.show();
	}
}