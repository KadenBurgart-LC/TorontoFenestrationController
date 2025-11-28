/*  fenestrationController.ino      (S0900.A.D.A.C0.A)
    
    This program is designed to run on the AutomationDirect P1AM-200 industrial controller.
    This program is intended to be uploaded to the P1AM-200 using the Arduino IDE, using the 
    instructions published here: https://facts-engineering.github.io/

    This program controls the automatic valves, relays, sensors, and other peripherals that 
    operate the fenestration wall in Oakville.
    The program should also expose a user interface over ethernet, accessed through a browser.

    This program references valves and relays based on the names given in the component list 
    S0900.A.D.1 and the system diagrams in S0900.A.D.2.

    This program hosts a webserver that is intended to pair with a specific web application.
    The user interface is all in the web application, which I am working on putting on an SD
    card that lives in the P1AM-200 module. I'll make a server interface to upload and download
    files to/from the SD card for over-the-air updates to the web app.

    Planning...
      What core processes do I need to run the wall?

      1) I need a process to read sensor values into some internal RAM data structure.
      2) I need a process to digest that RAM data structure into some form of output stream that goes to subscibers.
      2.1) I need a process to subscribe to the output stream and store the data in long-term memory, probably on an SD card.
      2.2) I need a process to subscribe to the output stream and send the data over serial. (maybe use the physical switch on the controller to disable the serial stream to save CPU time)
      NOTE: Do all that data stuff in a single thread right now. I want to avoid dynamic memory objects for the moment. We can do a whole queue/buffer subscription thing later maybe.
      No, not this: 5) I need a process to subscribe to the output stream and publish the data over a web API? No.
      5) I need a process to handle incoming ethernet requests and return what is needed, which may include the web app on first load, or more commonly a JSON object with a live data packet.
      6) I need a process to act as a high-level state machine that orchestrates everything else.
      7) I need a process to handle running through my outputs and making sure that they match their associated 'RAM commander variables'.
      8) I need a control (PID?) loop for the structural test pressure.
      9) I need s control (PID?) loop for the leakage pressure.

      Process 1) What do I need in my data structure?
        - Active readings from the pressure sensors.
        - Active readings from the linear transducers.
        - The states of my outputs.
        - The variables in my PID loop.

      Files?

    2025-11-10 - Kaden Burgart
    v0 - DRAFT
      Started the first draft of the program. We still don't have the valves and other components;
      just the controller, so the code is somewhat loose.
      Right now I just need to test the digital output module P1-15TD1 and the analog input
      modules P1-08ADL-1 and P1-08ADL-2.
      Should also test the ethernet module P1AM-ETH and the peripherals on the P1AM-200 including:
        -Real-Time Clock            PCF8563_RTC library
        -microSD card               SD library
        -SPI Flash
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

#include "HAL.h"                        // The hardware abstraction layer. Any code relating to a specific piece of hardware goes through here.
#include "lib_OSBos.h"                  // The custom library that acts as our simple kernel | WARNING: This is bypassing the HAL right now, using millis() and Serial.
#include "th_Blink.h"                   // A simple thread to blink an LED
#include "th_SerialConsole.h"           // Code relating to the serial console thread | WARNING: This is bypassing the HAL right now, using the Arduino Serial library AND digital outputs.
#include "th_WebServer.h"               // Code relating to the web server thread | WARNING: This is bypassing the HAL right now, using the Arduino Ethernet library.
//#include "th_DataLogger.h"              // Code relating to the data logging thread, which reads the state of the machine and saves to SD card and such

// Help us keep track of what version of the code is running
#define PROGRAM_VERSION "v0.1 - DRAFT"
#define PROGRAM_VERSION_DATE "2025-11-24"
#define PROGRAM_NAME "fenestrationController"
#define PROGRAM_STAMP "\n\n" PROGRAM_NAME "_" PROGRAM_VERSION " (" PROGRAM_VERSION_DATE ")\nBuild date: " __DATE__ " " __TIME__ "\n"

// Help us keep track of what threads we are running
#define T_BLINK_TEST 0
#define T_CONSOLE 1
#define T_WEBSVR 2

// This is the key tool that manages all of our multithreading
OSBos* kernel;

void setup() {
  HAL::init_Serial();
  Serial.println(PROGRAM_STAMP);  

  HAL::init_CPU();
  HAL::init_P1Slots();

  th_SerialConsole::initialize();
  th_WebServer::initialize();

  // set up an OSBos kernel with capacity for a certain number of threads
  kernel = new OSBos(10);
  
  kernel->Threads[T_BLINK_TEST]->RootMethod = th_Blink::tick;
  kernel->Threads[T_BLINK_TEST]->Active = true;
  kernel->Threads[T_BLINK_TEST]->ReadyPeriod_ms = 1000;

  kernel->Threads[T_CONSOLE]->RootMethod = th_SerialConsole::tick;
  kernel->Threads[T_CONSOLE]->Active = true;
  kernel->Threads[T_CONSOLE] ->ReadyPeriod_ms = 400;

  kernel->Threads[T_WEBSVR]->RootMethod = th_WebServer::tick;
  kernel->Threads[T_WEBSVR]->Active = true;
  kernel->Threads[T_WEBSVR]->ReadyPeriod_ms = 500;
}

void loop() {
  kernel->RunKernel();
}