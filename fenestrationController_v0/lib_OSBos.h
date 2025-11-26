/* OSBos.h
 *  Authored:    Kaden Burgart - 2020-04-13
 *  Description: This library contains a class that allows the programmer
 *               to quickly set up an almost-bare-metal OS for short, simple little
 *			         program threads (like OSB fragments).
 *
 *  Modified:    Kaden - 2025-11-17
 *  Notes:       Deadline alarm corrections.
 *
 *  How to...
 *    1) Initialize a thread
 *        Each thread needs a 'main' root function. Select an available thread index, and assign the root method.
 *        Set the thread to active.
 *        
 *        NOTE: Each thread will wail a certain amount of time between runs, as determined by ReadyPeriod_ms.
 *
 *        Ex.
 *          void t1_main() { ...do stuff... }
 *          ...
 *          osbOs->Threads[0]->RootMethod = t1_main;
 *          osbOs->Threads[0]->Active = true;
 */

#ifndef OSBos_h
#define OSBos_h

#include<Arduino.h>

typedef void (*Func) (void);

typedef struct Thread {
  bool Active; // The kernal will ignore any threads with this set to false
  Func RootMethod; // The main method for the thread
  unsigned int ReadyPeriod_ms; // The time that the kernel will wait between calls to this method
  unsigned int DeadlinePeriod_ms; // The maximum amount of time allowed between calls to this method, before triggering an alarm
  unsigned long MillisLastExecuted; // The time that the thread was last run
  unsigned long LastRuntime_us; // The duration that the thread hogged the CPU when last called
  unsigned long RuntimeAlarm_us; // If the thread runs longer than this, it will trigger an alarm

  bool RuntimeAlarmOn;
  bool DeadlineAlarmOn;
  bool RuntimeMonitoringOn;
  Func RuntimeAlarm; // Executed any time a method runs longer than expected
  Func DeadlineAlarm; // Executed any time a deadline is missed
};

class OSBos {
	public:
    Thread** Threads; // A pointer to an array of threads

    OSBos(
      unsigned char numThreads
      );

    void RunKernel(); // Monitor and run active threads

  private:
    unsigned char _numberOfThreads; // The maximum number of threads that can be managed with this OSBos instance
};

#endif