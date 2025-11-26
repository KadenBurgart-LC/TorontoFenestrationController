/* th_SerialConsole.h

This file contains the custom implementation of the serial console for this specific application.

This is where we write our different serial console commands.

*/

#ifndef CONSOLE_CUSTOMCOMMANDS
#define CONSOLE_CUSTOMCOMMANDS


namespace th_SerialConsole{
	// Must be run before ticking
	void initialize();

	// The main function to be called by the kernel
	void tick();
}

#endif