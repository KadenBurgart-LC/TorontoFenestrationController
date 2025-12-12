#ifndef THREAD_WEBSERVER
#define THREAD_WEBSERVER

#include <OSBos.h>      // The custom library that acts as our simple kernel https://github.com/actuvon/OSBos

namespace th_WebServer{
	// Must run before ticking
	void initialize();

	// The main function to be called by the kernel
	int8_t tick();

	inline Thread thread(tick, 100);
}

#endif