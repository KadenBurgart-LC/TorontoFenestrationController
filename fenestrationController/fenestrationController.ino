/*  fenestrationController.ino      (S0900.A.D.A.C0.A)

    NOTE: Look to the project's README.md file first.
    
    This program is designed to run on the AutomationDirect P1AM-200 industrial controller.
    This program is intended to be uploaded to the P1AM-200 using the Arduino IDE, using the 
    instructions published here: https://facts-engineering.github.io/

    This program controls the automatic valves, relays, sensors, and other peripherals that 
    operate the fenestration wall in Oakville.
    The program should be controlled using a web application.

    This program references valves and relays based on the names given in the component list 
    S0900.A.D.1 and the system diagrams in S0900.A.D.2.

    This program hosts a webserver that is intended to pair with a specific web application.
    The user interface is all in the web application, which I am working on putting on an SD
    card that lives in the P1AM-200 module. I'll make a server interface to upload and download
    files to/from the SD card for over-the-air updates to the web app.

    TODO...
      - I need a control (PID) loop for pressure control

    Initial draft: 2025-11-10 - Kaden Burgart

        -Real-Time Clock            PCF8563_RTC library
        -microSD card               SD library
        -Externak 2k-bit EEPROM     AT24MAC_EEPROM library
        -Watchdog timer             
        -Toggle Switch              digitalRead()
        -RGB LED                    Adafruit_NeoPixel library
        -Yellow LED                 digitalWrite()
        -SSL/HTTPS crypto           ArduinoECCX08 library

    2025-11-17 - Kaden
    v0 - DRAFT
      Added the serial console and OSBos.
      Added the DigitalOutput command.
      System planning and setting up infrastructure.
      Built the HELLO WORLD webserver, accessible over the network.

    2025-11-20 - Kaden
    v0.1 - DRAFT
      Refactoring for the application file structure.
      Continued development on the webserver.
      Added the hardware abstraction layer.

    2025-11-27 - Kaden
    v0.1 - DRAFT
      Moving versioning and update notes to GitHub.
      github.com/KadenBurgart-LC/TorontoFenestrationController
*/

#include <OSBos.h>                      // The custom library that acts as our simple kernel https://github.com/actuvon/OSBos
#include "HAL.h"                        // The hardware abstraction layer. Any code relating to a specific piece of hardware goes through here.
#include "th_test.h"                    // Making this to test a terminal async task with the webServer
#include "th_Blink.h"                   // A simple thread to blink an LED
#include "th_SerialConsole.h"           // Code relating to the serial console thread | WARNING: This is bypassing the HAL right now, using the Arduino Serial library AND digital outputs.
#include "th_WebServer.h"               // Code relating to the web server thread | WARNING: This is bypassing the HAL right now, using the Arduino Ethernet library.
#include "MechanicalSystem.h"           // The business logic of controlling our actuators on the wall
#include "th_DataLogger.h"              // Code relating to the data logging thread, which reads the state of the machine and saves to SD card and such
#include "lib_Utils.h"

// Help us keep track of what version of the code is running
#define PROGRAM_VERSION "v0.3 - DRAFT"
#define PROGRAM_VERSION_DATE "2025-12-17"
#define PROGRAM_NAME "fenestrationController"
#define PROGRAM_STAMP "\n\n" PROGRAM_NAME "_" PROGRAM_VERSION " (" PROGRAM_VERSION_DATE ")\nBuild date: " __DATE__ " " __TIME__ "\n"

// This is the key tool that manages all of our multithreading
OSBos kernel(10);

void setup() {
    lib_Util::WatermarkStackRam();

    HAL::init_Serial();
    Serial.println(PROGRAM_STAMP);  
    HAL::init_CPU();
    HAL::init_P1Slots();

    th_SerialConsole::initialize();
    th_WebServer::initialize();

    // Continuous Runtime Tasks
    kernel.AddThread(th_Blink::thread);
    kernel.AddThread(th_SerialConsole::thread);
    kernel.AddThread(th_WebServer::thread);
    kernel.AddThread(th_DataLogger::thread);

    // Terminal Async Task Initialization
    kernel.AddThread(MechanicalSystem::tk_StopAll::Task);
    kernel.AddThread(MechanicalSystem::tk_SetLowPressure_Positive::Task);
    kernel.AddThread(MechanicalSystem::tk_SetLowPressure_Negative::Task);
    kernel.AddThread(MechanicalSystem::tk_SetHighPressure_Positive::Task);
    kernel.AddThread(MechanicalSystem::tk_SetHighPressure_Negative::Task);
    kernel.AddThread(th_test::thread);                      // Used for the example button widget

    th_DataLogger::writeToLog("The system is booting up...");
}

void loop() {
    kernel.RunKernel();
}