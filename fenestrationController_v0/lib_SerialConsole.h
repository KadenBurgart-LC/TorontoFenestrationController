/* SerialConsole.h
 *  Authored:    Kaden Burgart - 2020-04-13
 *  Description: This library contains a class that allows the programmer
 *               to quickly set up a command console on the arduino.
 *
 *  WRITTEN FOR LABTEST CERTIFICATION
 *
 *  Modified:    Kaden Burgart - 2020-04-14
 *  Notes:	   	 Misc improvements.
 *  
 *  How to...
 *    1) Initialize an object
 *        Determine the appropriate max limits for the size buffers for your application.
 *        Check the class constructor and the exploded private variable descriptions below for a list of variables and their purposes.
 *        In my .ino code, I declare a pointer to a SerialConsole object, and then I instantiate with the "new" keyword.
 *        Ex. 
 *          SerialConsole* console;
 *          ...
 *          console = new SerialConsole(5, 30, 10, 10, 10, 500);
 *          
 *    2) Initialize a command
 *        To do this, a trigger word must be chosen, and a void function must be written.
 *        Functions and their triggers are stored in arrays within the SerialConsole class.
 *        A particular trigger corresponds to the function at the same array index.
 *        Ex.
 *          void console_test(){ ...do stuff... }
 *          ...
 *          strcpy(console->Triggers[0], "test");
 *          console->Functions[0] = console_test;
 *          
 *    3) Access command arguments
 *        When a command has been executed from the serial monitor, the parameters that it was called with are needed in the
 *        function associated with the command trigger. The Arguments array is a public object that gets populated with the
 *        arguments from the console whenever a command is run. It can be accessed like so:
 *          void console_test(){
 *            Serial.println("-------------TEST----------------");
 *            Serial.println(console->Arguments[0]);
 *            Serial.println(console->Arguments[1]);
 *          }
 */

#ifndef SerialConsole_h
#define SerialConsole_h

#include <Arduino.h>

typedef void (*Func) (void);

class SerialConsole {
	public:
		char** Arguments; // A pointer to the array of arguments
    	char** HelpMsg; // A pointer to the array of help messages

		char** Triggers; // A pointer to the array of command string triggers
		Func* Functions; // A pointer to the array of command functions

		SerialConsole(
			unsigned char numCommands, 
			unsigned char lineLength,
			unsigned char cmdLength,
			unsigned char argLength,
			unsigned char maxArgs,
			unsigned long scanPeriod
			);

		void Listen(); // Check the serial port for traffic. Run commands if applicable.

	private:
		unsigned char _numberOfCommands;
		unsigned char _instructionLineMaxLength; // The maximum number of characters allowed in a single instruction line
		unsigned char _commandMaxLength; // The maximum number of characters allowed for the command trigger
		unsigned char _argMaxLength; // The maximum number of characters allowed for a single argument
		unsigned char _maxNumArgs; // The maximum number of arguments allowed in a single instruction line
		unsigned long _minScanPeriod; // The minimum number of milliseconds between scans of the serial line

		unsigned long _lastScanMillis; // The millis() timestamp of the last scan
		char* _commandBuffer; // Temporarilly holds incomming commands
};

#endif
