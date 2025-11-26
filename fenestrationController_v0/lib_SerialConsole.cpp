#include "Arduino.h"
#include "lib_SerialConsole.h"

SerialConsole::SerialConsole(
			unsigned char numCommands, 
			unsigned char lineLength,
			unsigned char cmdLength,
			unsigned char argLength,
			unsigned char maxArgs,
			unsigned long scanPeriod) {
	
	_numberOfCommands = numCommands;
	_instructionLineMaxLength = lineLength;
	_commandMaxLength = cmdLength;
	_argMaxLength = argLength;
	_maxNumArgs = maxArgs;
	_minScanPeriod = scanPeriod;

	Triggers = (char**)malloc(numCommands * sizeof(char*));
  HelpMsg = (char**)malloc(numCommands * sizeof(char*)); // Only make room for pointers, not data. Declare data as a constant, and point this to it.
	Functions = (Func*)malloc(numCommands * sizeof(Func*));
	for(int i=0; i<numCommands; i++){
		Triggers[i]  = (char*)malloc(cmdLength + 1);
    Triggers[i][0] = '\0';

    // Do not allocate space for help messages. That should be allocated outside the class.
    HelpMsg[i] = 0;

		Functions[i] = (Func)malloc(sizeof(Func));
	} 

	_commandBuffer = (char*)malloc(lineLength + 1);
	Arguments = (char**)malloc(maxArgs * sizeof(char*));
	for(int i=0; i<maxArgs; i++) Arguments[i] = (char*)malloc(argLength + 1);
}

void SerialConsole::Listen(){
	unsigned long now = millis();

	if(now - _lastScanMillis > _minScanPeriod){
		if(Serial.available() > 0){
			int index = -1;
			while(Serial.available()){
				index++;
				_commandBuffer[index] = Serial.read();
				if(index >= _instructionLineMaxLength-1)break;
				if(_commandBuffer[index] == '\n' || _commandBuffer[index] == '\r'){
					_commandBuffer[index] = '\0';
					break;
				}
			}
			_commandBuffer[++index] = '\0';
			Serial.print("\n>> ");Serial.println(_commandBuffer);

			for(int i=0;i<_maxNumArgs;i++){ strcpy(Arguments[i], ""); }

			// Parse arguments
			char* arg = strtok(_commandBuffer, " ");
			for(int i = 0; i < _maxNumArgs; i++){
				if(arg != 0){
					strcpy(Arguments[i], arg);
				}
				else{
					break;
				}
				arg = strtok(NULL, " ");
			}

			//Convert to lower case
			for(int i=0; i<_maxNumArgs; i++){
				for(int j=0; j<_argMaxLength; j++){
					if(Arguments[i][j] == '\0')break;
					else{
						Arguments[i][j] = tolower(Arguments[i][j]);
					}
				}
			}

      bool cmdFound = false;
      if(strcmp(Arguments[0], "help") == 0){
      	if(strcmp(Arguments[1], "") == 0){
      		Serial.println("Type help <command> for help on a specific command.");
      	}
        else{
        	for(int i=0; i<_numberOfCommands; i++){
	          if(strcmp(Arguments[1], Triggers[i]) == 0) {
	            cmdFound = true;
	            if(HelpMsg[i] != 0) Serial.println(HelpMsg[i]);
	            else{
	            	Serial.print("Console: No help message available for command ");
	            	Serial.print(i);
	            	Serial.print(": ");
	            	Serial.println(Triggers[i]);
	            }
	          }
	        }
        }
      }
      else{
  			for(int i=0; i<_numberOfCommands; i++){
  				if(strcmp(Arguments[0], Triggers[i]) == 0){
            cmdFound = true;

            if(Functions[i] == 0) Serial.println("\nConsole: A function with an existing trigger was called, but there is no function associated with the trigger.");
            else (Functions[i])();
          }
  			}
      }
      if(!cmdFound){
        Serial.println("\nConsole: Command not recognized.");
        Serial.println("Available commands:");
        for(int i=0; i<_numberOfCommands; i++){
          if(strcmp(Triggers[i], "") != 0){
            Serial.print(" - ");Serial.println(Triggers[i]);
          }
        }
        Serial.println("To get help info for a command, type ""help <command>""");
      }
		}

		_lastScanMillis = millis();
	}
}
