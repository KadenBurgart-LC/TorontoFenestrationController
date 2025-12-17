#ifndef THREAD_DATALOGGER
#define THREAD_DATALOGGER

#include <OSBos.h>      // The custom library that acts as our simple kernel https://github.com/actuvon/OSBos
#include <Arduino.h>    // Used for strings

namespace th_DataLogger{

	// Must run before ticking
	//void initialize(); // I don't think we need this?

	// The main function to be called by the kernel
	int8_t tick();

	inline Thread thread(tick, 1000);

	// Add a new "standard row" to the data log
	uint8_t logStandardDataRow();

	// Add some special note to the data log. Let's... put this note in a seperate column at the end of the normal log data to make data processing easier for our poor technicians
	uint8_t writeToLog(String str);

	// Return the last line which was logged
	String getLastLogLine();

	String getCurrentLogFilePath();
}

#endif

/*

Ok how do I want this thing to work....

* The logger's own tick thread will run once per second and it will log a data row, by calling 
some logStandardDataRow() function or something.

* We will also be able to log custom non-standard rows with some writeToLog(string) function.

* Each row logged will go to a file on the SD card. The log files will be split up day-by-day, and
the logger will automatically use the log file according to today's date on the RTC.

* Each row logged will have a timestamp as the first entry in the row. Probably a simple uint16_t.
I might do this as raw RTC seconds, and possibly another one or two digits for us to increment so that
the numbers in this field stay unique even if there are 9 or 99 log entries per second.

* We will keep rolling buffer (maybe 20) of the most recent logs made, so that people can cal a
getLogsSince(uint16_t timestamp) function. This allows the WebApp to say "hey, fill me in on everything
that happened since the last time I asked", so that the WebApp knows it's not missing anything.
BUT this will only get the last 20 loggs, and it won't go to the SD card becuase that will require
more clever work with Streams. I want this function to use simple Strings or char*.

* We want a getLastLogLine() function so that the application can ask the logger what's going on.

* We will need functions to turn the logger on and off, and to check whether it is currently active.

* We will need some kind of status indicator to tell us if everything is OK, or if we are experiencing
some sort of error.

* We will NOT need a function like streamArchivedLogFile(YYYY, MM, DD) so that we can download the logs
from past days. Don't use this. INSTEAD: We will be needing a way of serving SD card files through the 
WebServer anyway, so use that interface to do this job.

* We NOT need streamLogsSince(timestamp) function. If the user needs data from earlier in the day,
make them download the whole day's log file using the WebServer->ServeSdFile thingy.



*/