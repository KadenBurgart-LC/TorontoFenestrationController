#ifndef THREAD_WEBSERVER
#define THREAD_WEBSERVER

#include "lib_OSBos.h"

namespace th_WebServer{
	// Must run before ticking
	void initialize();

	// The main function to be called by the kernel
	int8_t tick();

	inline Thread thread(tick, 100);
}

#endif