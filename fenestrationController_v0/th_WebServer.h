#ifndef THREAD_WEBSERVER
#define THREAD_WEBSERVER

namespace th_WebServer{
	// Must run before ticking
	void initialize();

	// The main function to be called by the kernel
	void tick();
}

#endif