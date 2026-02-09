#include "th_SerialConsole.h"

#include "HAL.h"
#include "th_DataLogger.h"
#include <SerialConsole.h>  // https://github.com/actuvon/SerialConsole
#include <P1AM.h>      // The public library for the AutomationDirect controller we are using  https://github.com/facts-engineering/P1AM
#include "lib_Utils.h" // Used to help me spy on my RAM

// Private members
namespace {
  SerialConsoleConfig setupConsoleConfig(){
    SerialConsoleConfig config;

    config.numCommands = 40;
    config.maxFullLineLength = 100;
    config.maxArgLength = 80;
    config.maxNumArgs = 8;

    return config;
  }

  SerialConsoleConfig ConsoleConfig = setupConsoleConfig();
  SerialConsole console(ConsoleConfig);

  void c_DiscreteOutput(){
    uint8_t slot    = strtol(console.Arguments[1], NULL, 10);
    uint8_t channel = strtol(console.Arguments[2], NULL, 10);
    uint8_t state   = strtol(console.Arguments[3], NULL, 10);

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
    uint8_t slot    = strtol(console.Arguments[1], NULL, 10);
    uint8_t channel = strtol(console.Arguments[2], NULL, 10);

    int inputCounts = 0;

    Serial.print("Attempting to read channel ");
    Serial.print(channel);
    Serial.print(" on slot ");
    Serial.print(slot);
    Serial.println("...");

    if(
      (5 < slot && slot < 7) &&
      (0 < channel && channel < 9)
      ){
      float inputVolts = 0;

      inputCounts = P1.readAnalog(slot, channel);
      inputVolts = 10 * ((float)inputCounts / 8191); // Conversion formula from the P1-08ADL-2 documentation https://facts-engineering.github.io/

      Serial.print("Channel voltage: ");
      Serial.print(inputVolts);
      Serial.println(" V");
    }
    else if(
      (3 < slot && slot < 6) &&
      (0 < channel && channel < 9)
      ){
      float input_mA = 0;

      inputCounts = P1.readAnalog(slot, channel);
      input_mA = 20 * ((float)inputCounts / 8191); // Conversion formula from the P1-08ADL-1 documentation https://facts-engineering.github.io/

      Serial.print("Channel current: ");
      Serial.print(input_mA);
      Serial.println(" mA");
    }
    else{
      Serial.println("Invalid slot or channel.");
    }
  }

  void c_AnalogOutput(){
    // https://facts-engineering.github.io/modules/P1-08DAL-1/P1-08DAL-1.html

    uint8_t slot    = strtol(console.Arguments[1], nullptr, 10);
    uint8_t channel = strtol(console.Arguments[2], nullptr, 10);
    float value = strtof(console.Arguments[3], nullptr);

    int outputCounts = 0;

    if(
      (1 < slot && slot < 4) && 
      (0 < channel && channel < 9)
      ){
      if(4 <= value && value <= 20){
        outputCounts = (value - 4) * 4095 / 16; // Convert from mA to 12 bit (4095) counts 

        Serial.print("Attempting to write");
        Serial.print(value);
        Serial.print(" mA to channel ");
        Serial.print(channel);
        Serial.print(" on slot ");
        Serial.print(slot);
        Serial.println("...");

        P1.writeAnalog(outputCounts, slot, channel);
      }
      else {
        Serial.println("Analog current output channels must only write values in the range of 4-20 mA.\nCancelling operation.");
      }      
    }
    else {
      Serial.println("Invalid slot or channel");
    }
  }

  void c_P1StatusCodes(){
    uint8_t slot    = strtol(console.Arguments[1], NULL, 10);

    char statusCodeBuffer[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    char configCodeBuffer[2] = {0,0};

    if(slot == 6){
      Serial.print("Checking status codes on P1-08ADL-2 module on slot ");
      Serial.print(slot);
      Serial.println("...");

      P1.readModuleConfig(configCodeBuffer, slot);
      P1.readStatus(statusCodeBuffer, slot);

      uint16_t configCode = (configCodeBuffer[0] << 8) | (configCodeBuffer[1]);

      Serial.print("Config code: ");Serial.println(configCode);
      Serial.print("Status code: Lost 24V: ");Serial.println((uint8_t)statusCodeBuffer[3]);
      Serial.print("Status code: Under range: ");Serial.println((uint8_t)statusCodeBuffer[7]);
      Serial.print("Status code: Over range: ");Serial.println((uint8_t)statusCodeBuffer[11]);

      if(configCode == 0x4000) Serial.println("Config: Channel 1 is enabled.");
      if(configCode == 0x4001) Serial.println("Config: Channels 1 - 2 are enabled.");
      if(configCode == 0x4002) Serial.println("Config: Channels 1 - 3 are enabled.");
      if(configCode == 0x4003) Serial.println("Config: Channels 1 - 4 are enabled.");
      if(configCode == 0x4004) Serial.println("Config: Channels 1 - 5 are enabled.");
      if(configCode == 0x4005) Serial.println("Config: Channels 1 - 6 are enabled.");
      if(configCode == 0x4006) Serial.println("Config: Channels 1 - 7 are enabled.");
      if(configCode == 0x4007) Serial.println("Config: Channels 1 - 8 are enabled.");

      if((statusCodeBuffer[3] & 0x01) ==  1) Serial.println("Flag: Module diagnostics failure.");
      if((statusCodeBuffer[3] & 0x02) ==  2) Serial.println("Flag: Lost 24V error.");
      if((statusCodeBuffer[7] & 0x01) == 1) Serial.println("Flag: Under range error on channel 1");
      if((statusCodeBuffer[7] & 0x02) == 2) Serial.println("Flag: Under range error on channel 2");
      if((statusCodeBuffer[7] & 0x04) == 4) Serial.println("Flag: Under range error on channel 3");
      if((statusCodeBuffer[7] & 0x08) == 8) Serial.println("Flag: Under range error on channel 4");
      if((statusCodeBuffer[11] & 0x01) == 1) Serial.println("Flag: Over range error on channel 1");
      if((statusCodeBuffer[11] & 0x02) == 2) Serial.println("Flag: Over range error on channel 2");
      if((statusCodeBuffer[11] & 0x04) == 4) Serial.println("Flag: Over range error on channel 3");
      if((statusCodeBuffer[11] & 0x08) == 8) Serial.println("Flag: Over range error on channel 4");
    }
    else if (3 < slot && slot < 6){
      Serial.print("Checking status codes on P1-08ADL-1 module on slot ");
      Serial.print(slot);
      Serial.println("...");

      P1.readModuleConfig(configCodeBuffer, slot);
      P1.readStatus(statusCodeBuffer, slot);

      uint16_t configCode = (configCodeBuffer[0] << 8) | (configCodeBuffer[1]);

      Serial.print("Config code: ");Serial.println(configCode);
      Serial.print("Status code: Lost 24V: ");Serial.println((uint8_t)statusCodeBuffer[3]);
      Serial.print("Status code: Under range: ");Serial.println((uint8_t)statusCodeBuffer[7]);
      Serial.print("Status code: Over range: ");Serial.println((uint8_t)statusCodeBuffer[11]);

      if(configCode == 0x4000) Serial.println("Config: Channel 1 is enabled.");
      if(configCode == 0x4001) Serial.println("Config: Channels 1 - 2 are enabled.");
      if(configCode == 0x4002) Serial.println("Config: Channels 1 - 3 are enabled.");
      if(configCode == 0x4003) Serial.println("Config: Channels 1 - 4 are enabled.");
      if(configCode == 0x4004) Serial.println("Config: Channels 1 - 5 are enabled.");
      if(configCode == 0x4005) Serial.println("Config: Channels 1 - 6 are enabled.");
      if(configCode == 0x4006) Serial.println("Config: Channels 1 - 7 are enabled.");
      if(configCode == 0x4007) Serial.println("Config: Channels 1 - 8 are enabled.");

      if((statusCodeBuffer[3] & 0x01) ==  1) Serial.println("Flag: Module diagnostics failure.");
      if((statusCodeBuffer[3] & 0x02) ==  2) Serial.println("Flag: Lost 24V error.");
      if((statusCodeBuffer[7] & 0x01) ==  1) Serial.println("Flag: Under range error on channel 1");
      if((statusCodeBuffer[7] & 0x02) ==  2) Serial.println("Flag: Under range error on channel 2");
      if((statusCodeBuffer[7] & 0x04) ==  4) Serial.println("Flag: Under range error on channel 3");
      if((statusCodeBuffer[7] & 0x08) ==  8) Serial.println("Flag: Under range error on channel 4");
      if((statusCodeBuffer[7] & 0x10) == 16) Serial.println("Flag: Under range error on channel 5");
      if((statusCodeBuffer[7] & 0x20) == 32) Serial.println("Flag: Under range error on channel 6");
      if((statusCodeBuffer[7] & 0x40) == 64) Serial.println("Flag: Under range error on channel 7");
      if((statusCodeBuffer[7] & 0x80) ==128) Serial.println("Flag: Under range error on channel 8");
      if((statusCodeBuffer[11] & 0x01) ==  1) Serial.println("Flag: Over range error on channel 1");
      if((statusCodeBuffer[11] & 0x02) ==  2) Serial.println("Flag: Over range error on channel 2");
      if((statusCodeBuffer[11] & 0x04) ==  4) Serial.println("Flag: Over range error on channel 3");
      if((statusCodeBuffer[11] & 0x08) ==  8) Serial.println("Flag: Over range error on channel 4");
      if((statusCodeBuffer[11] & 0x10) == 16) Serial.println("Flag: Over range error on channel 5");
      if((statusCodeBuffer[11] & 0x20) == 32) Serial.println("Flag: Over range error on channel 6");
      if((statusCodeBuffer[11] & 0x40) == 64) Serial.println("Flag: Over range error on channel 7");
      if((statusCodeBuffer[11] & 0x80) ==128) Serial.println("Flag: Over range error on channel 8");
    }
    else if (slot == 1){
      Serial.println("P1-15TD1 Discrete Sink Output modules do not provide any status data.");
    }
    else if(1 < slot && slot < 4){
      Serial.print("Checking status codes on P1-08DAL-1 module on slot ");
      Serial.print(slot);
      Serial.println("...");

      P1.readStatus(statusCodeBuffer, slot);

      Serial.print("Status code: Lost 24V: ");Serial.println((uint8_t)statusCodeBuffer[3]);

      if((statusCodeBuffer[3] & 0x02) == 2) Serial.println("Flag: Lost 24V error.");
      if((statusCodeBuffer[3] & 0x01) == 1) Serial.println("Flag: Module diagnostics failure.");
    }
    else{
      Serial.print("The selected slot (");
      Serial.print(slot);
      Serial.println(") is not currently supported by this command.");
    }
  }

  void c_LED(){
    uint8_t R = strtol(console.Arguments[1], NULL, 10);
    uint8_t G = strtol(console.Arguments[2], NULL, 10);
    uint8_t B = strtol(console.Arguments[3], NULL, 10);

    HAL::set_C0_1_RgbLed(R, G, B);
  }

  // List files in a directory on the SD card
  void c_SD_ls(){
    HAL::SD_PrintDirectory(Serial, console.Arguments[1]);
  }

  // Dump the contents of a file from the SD card into the Serial interface
  void c_SD_cat(){
    bool success = HAL::SD_PrintFileContents(Serial, console.Arguments[1]);

    if(!success) Serial.println("Failed to find and print the file.");
  }

  // Write an argument into a file on the SD card (overwriting any existing file)
  void c_SD_write(){
    bool success = HAL::SD_WriteFile(console.Arguments[2], console.Arguments[1]);

    if(success) Serial.println("File created.");
    else Serial.println("Failed to write the file.");
  }

  // Delete a file from the SD card
  void c_SD_rm(){
    bool success = HAL::SD_DeleteFile(console.Arguments[1]);

    if(success) Serial.println("File deleted.");
    else Serial.println("Deletion failed.");
  }

  void c_SD_append(){
    bool success = HAL::SD_AppendFile(console.Arguments[2], console.Arguments[1]);

    if(success) Serial.println("Data appended successfully.");
    else Serial.println("Failed to append data.");
  }

  // Set the date and time on the RTC
  void c_RTC_set(){
    /* Prameters must be as follows...
         arg[1] = "Jan"      - This needs to be a 3 letter code with the first letter capitalized and the last two letters lower-case
         arg[2] = "12"       - This needs to be two characters that form the day of the month
         arg[3] = "2025"     - This needs to be four characters that form the day of the year
         arg[4] = "Fri"      - This needs to be a 3 letter code with the first letter capitalized and the last two letters lower-case
         arg[5] = "08:00:00" - HH:MM:SS exactly
     */
    // String date = String(console.Arguments[1]) + " " + String(console.Arguments[2]) + " " + String(console.Arguments[3]) + " " + String(console.Arguments[4]);
    // String time = String(console.Arguments[5]);

    // HAL::RTC_SetDateTime(date, time);

    uint16_t year = atoi(console.Arguments[1]);
    uint8_t month = atoi(console.Arguments[2]);
    uint8_t day = atoi(console.Arguments[3]);
    uint8_t hour = atoi(console.Arguments[4]);
    uint8_t minute = atoi(console.Arguments[5]);
    uint8_t second = atoi(console.Arguments[6]);

    HAL::RTC_SetDateTime(year, month, day, hour, minute, second);
  }

  void c_RTC_get(){
    Serial.println(HAL::RTC_GetDateTime());
  }

  void c_Log_Line(){
    bool success = th_DataLogger::writeToLog(console.Arguments[1]);
    Serial.println(th_DataLogger::getLastLogLine());
    if(!success) Serial.println("Write action failed.");
  }

  void c_LogFile(){
    Serial.println(th_DataLogger::getCurrentLogFilePath());
  }

  void c_LogsSince(){
    uint64_t timestamp = atol(console.Arguments[1]);
  }

  void c_StackPrints(){
    lib_Util::PrintRamStackUseageToSerial();
  }

  void c_RamNow(){
    lib_Util::PrintRamStatusToSerial();
  }

  void c_RamAdr(){
    lib_Util::PrintRamAddresses();
  }
}

namespace th_SerialConsole{
  void initialize(){
    console.AddCommand("p1s", c_P1StatusCodes, "Check the status codes on the P1 modules.\np1s <slot>");
    console.AddCommand("do", c_DiscreteOutput, "Digital Output command.\nTurn a digital output on or off.\ndo <slot> <channel> <state>");
    console.AddCommand("ai", c_AnalogInput, "Analog Input command.\nRead an analog input.\nai <slot> <channel>");
    console.AddCommand("ao", c_AnalogOutput, "Analog Output command.\nWrite an analog value to a P1 module (in mA or V).\nao <slot> <channel> <value>");
    console.AddCommand("pm", []() { P1.printModules();}, "Print out all installed P1 modules.");
    console.AddCommand("led", c_LED, "Change the setting on the RGB LED on the CPU.\nEach color has a min of 0 and a max of 255.\nled <R> <G> <B>");
    console.AddCommand("sdls", c_SD_ls, "List the contents of a directory on the SD card (root is \"/\").\nsdls <dir>");
    console.AddCommand("sdcat", c_SD_cat, "Dump the contents of a file from the SD card.\nsdcat <filePath>");
    console.AddCommand("sdwrite", c_SD_write, "Write a file to the SD card, overwriting it if it exists.\nsdwrite <filePath> <dataToWrite>\nNote that there can't be spaces in the string, since that's the argument delimiter!");
    console.AddCommand("sdrm", c_SD_rm, "Delete a file from the SD card.\nsdrm <filePath>");
    console.AddCommand("sdappend", c_SD_append, "Append data to a file.\nsdappend <filePath> <dataToAppend>\nNote that there can't be spaces in the string, since that's the argument delimiter!");
    console.AddCommand("RTCget", c_RTC_get, "Print the current date and time according to the Real-Time Clock");
    console.AddCommand("RTCset", c_RTC_set, "Set the date and time on the RTC.\nRTCset <YYYY> <MM> <DD> <HH> <MM> <SS>\nExample: RTCset 2025 12 05 15 21 00");
    console.AddCommand("logLine", c_Log_Line, "Add a line with a custom description to the day's log file, and show the full line in the console.\nlogLine <customTextToWrite>");
    console.AddCommand("logFile", c_LogFile, "What is the path of the current log file?");
    console.AddCommand("logsSince", c_LogsSince, "Get all logs since a certain timestamp.\nlogsSince <timestamp>\n<timestamp> is a uint64_t UNIX timestamp.");
    console.AddCommand("stackPrints", c_StackPrints, "Check and see how much stack RAM we have trampled over throughout the lifetime of the program.");
    console.AddCommand("ramNow", c_RamNow, "How much RAM are we using right now?");
    console.AddCommand("ramAdr", c_RamAdr, "Print the addresses of where things are in RAM.");
    console.AddCommand("test", c_Test);
  }

  int8_t tick(){
    console.Listen();

    return 0; // Tell OSBos to keep running
  }
}