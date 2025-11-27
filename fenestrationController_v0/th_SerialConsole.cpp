#include "th_SerialConsole.h"

#include "HAL.h"
#include "lib_SerialConsole.h"
#include <P1AM.h>      // The public library for the AutomationDirect controller we are using

// Private members
namespace {
  SerialConsole* console = nullptr;

  void c_DiscreteOutput(){
    uint8_t slot    = strtol(console->Arguments[1], NULL, 10);
    uint8_t channel = strtol(console->Arguments[2], NULL, 10);
    uint8_t state   = strtol(console->Arguments[3], NULL, 10);

    Serial.print("Attempting to write ");
    Serial.print(state);
    Serial.print(" to channel ");
    Serial.print(channel);
    Serial.print(" on slot ");
    Serial.print(slot);
    Serial.println("...");

    if(
      (0 < slot && slot < 2) &&
      (0 < channel && channel < 16) &&
      (-1 < state && state < 2)
      ){
      P1.writeDiscrete(state, slot, channel);
    }
    else{
      Serial.println("Invalid state, slot, or channel.");
    }
  }

  void c_Test(){
    Serial.println("Console test triggered");
  }

  void c_AnalogInput(){
    uint8_t slot    = strtol(console->Arguments[1], NULL, 10);
    uint8_t channel = strtol(console->Arguments[2], NULL, 10);

    int inputCounts = 0;
    float inputVolts = 0;

    Serial.print("Attempting to read channel ");
    Serial.print(channel);
    Serial.print(" on slot ");
    Serial.print(slot);
    Serial.println("...");

    if(
      (2 < slot && slot < 4) &&
      (0 < channel && channel < 9)
      ){
      inputCounts = P1.readAnalog(slot, channel);
      inputVolts = 10 * ((float)inputCounts / 8191); // Conversion formula from the P1-08ADL-2 documentation

      Serial.print("Channel voltage: ");
      Serial.print(inputVolts);
      Serial.println(" V");
    }
    else{
      Serial.println("Invalid slot or channel.");
    }
  }

  void c_P1StatusCodes(){
    uint8_t slot    = strtol(console->Arguments[1], NULL, 10);

    char statusCodeBuffer[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    char configCodeBuffer[2] = {0,0};

    if(slot == 3){
      Serial.print("Checking status codes on P1-08ADL-2 module on slot ");
      Serial.print(slot);
      Serial.println("...");

      P1.readModuleConfig(configCodeBuffer, slot);
      P1.readStatus(statusCodeBuffer, slot);

      uint16_t configCode = (configCodeBuffer[0] << 8) | (configCodeBuffer[1]);

      Serial.print("Slot 3: Config code: ");Serial.println(configCode);
      Serial.print("Slot 3: Status code: Lost 24V: ");Serial.println((uint8_t)statusCodeBuffer[3]);
      Serial.print("Slot 3: Status code: Under range: ");Serial.println((uint8_t)statusCodeBuffer[7]);
      Serial.print("Slot 3: Status code: Over range: ");Serial.println((uint8_t)statusCodeBuffer[11]);

      if(configCode == 0x4000) Serial.println("Config: Channel 1 is enabled.");
      if(configCode == 0x4001) Serial.println("Config: Channels 1 - 2 are enabled.");
      if(configCode == 0x4002) Serial.println("Config: Channels 1 - 3 are enabled.");
      if(configCode == 0x4003) Serial.println("Config: Channels 1 - 4 are enabled.");
      if(configCode == 0x4004) Serial.println("Config: Channels 1 - 5 are enabled.");
      if(configCode == 0x4005) Serial.println("Config: Channels 1 - 6 are enabled.");
      if(configCode == 0x4006) Serial.println("Config: Channels 1 - 7 are enabled.");
      if(configCode == 0x4007) Serial.println("Config: Channels 1 - 8 are enabled.");

      if((statusCodeBuffer[3] & 0x03) == 2) Serial.println("Flag: Lost 24V error.");
      if((statusCodeBuffer[7] & 0x01) == 1) Serial.println("Flag: Under range error on channel 1");
      if((statusCodeBuffer[7] & 0x02) == 2) Serial.println("Flag: Under range error on channel 2");
      if((statusCodeBuffer[7] & 0x04) == 4) Serial.println("Flag: Under range error on channel 3");
      if((statusCodeBuffer[7] & 0x08) == 8) Serial.println("Flag: Under range error on channel 4");
      if((statusCodeBuffer[11] & 0x01) == 1) Serial.println("Flag: Over range error on channel 1");
      if((statusCodeBuffer[11] & 0x02) == 2) Serial.println("Flag: Over range error on channel 2");
      if((statusCodeBuffer[11] & 0x04) == 4) Serial.println("Flag: Over range error on channel 3");
      if((statusCodeBuffer[11] & 0x08) == 8) Serial.println("Flag: Over range error on channel 4");
    }
    else{
      Serial.print("The selected slot: ");
      Serial.print(slot);
      Serial.println(" is not currently supported by this command.");
    }
  }

  void c_LED(){
    uint8_t R = strtol(console->Arguments[1], NULL, 10);
    uint8_t G = strtol(console->Arguments[2], NULL, 10);
    uint8_t B = strtol(console->Arguments[3], NULL, 10);

    HAL::set_C0_1_RgbLed(R, G, B);
  }

  // List files in a directory on the SD card
  void c_SD_ls(){
    HAL::SD_PrintDirectory(Serial, console->Arguments[1]);
  }

  // Dump the contents of a file from the SD card into the Serial interface
  void c_SD_cat(){
    HAL::SD_PrintFileContents(Serial, console->Arguments[1]);
  }
}

namespace th_SerialConsole{
  void initialize(){
    // set up the serial console
    console = new SerialConsole(
        15, // number of commands
        50, // maximum character count of any full command line (with arguments)
        15, // maximum character count of any command
        10, // maximum character count of any single argument
        5,  // maximum number of arguments allowed in a single command line
        1   // the scan period (set to 1ms to let OSBos control this)
      );

    strcpy(console->Triggers[0], "test");
    console->Functions[0] = c_Test;

    strcpy(console->Triggers[1], "do");
    console->Functions[1] = c_DiscreteOutput;
    console->HelpMsg[1] = "Digital Output command.\nTurn a digital output on or off.\ndo <slot> <channel> <state>";

    strcpy(console->Triggers[2], "ai");
    console->Functions[2] = c_AnalogInput;
    console->HelpMsg[2] = "Analog Input command.\nRead an analog input.\nai <slot> <channel>";

    strcpy(console->Triggers[3], "pm");
    console->Functions[3] = []() { P1.printModules(); };
    console->HelpMsg[3] = "Print out all installed P1 modules.";

    console->Triggers[4] = "p1s";
    console->Functions[4] = c_P1StatusCodes;
    console->HelpMsg[4] = "Check the status codes on the P1 modules.\np1s <slot>";

    console->Triggers[5] = "led";
    console->Functions[5] = c_LED;
    console->HelpMsg[5] = "Change the setting on the RGB LED on the CPU.\nEach color has a min of 0 and a max of 255.\nled <R> <G> <B>";
  
    console->Triggers[6] = "sdls";
    console->Functions[6] = c_SD_ls;
    console->HelpMsg[6] = "List the contents of a directory on the SD card (root is \"/\").\nsdls <dir>";
    
    console->Triggers[7] = "sdcat";
    console->Functions[7] = c_SD_cat;
    console->HelpMsg[7] = "Dump the contents of a file from the SD card.\nsdcat <filePath>";
  }

  void tick(){
    console->Listen();
  }
}