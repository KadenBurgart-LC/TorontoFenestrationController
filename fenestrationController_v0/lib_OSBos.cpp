#include "Arduino.h"
#include "lib_OSBos.h"

OSBos::OSBos(unsigned char numThreads){
  _numberOfThreads = numThreads;

  Threads = (Thread**)malloc(numThreads * sizeof(Thread*));
  for(int i=0; i<_numberOfThreads; i++){
    Threads[i] = (Thread*)malloc(sizeof(Thread));

    Threads[i]->Active = false;
    Threads[i]->RootMethod = 0;
    Threads[i]->ReadyPeriod_ms = 0;
    Threads[i]->DeadlinePeriod_ms = 0;
    Threads[i]->LastRuntime_us = 0;
    Threads[i]->RuntimeAlarm_us = 0;
    Threads[i]->RuntimeAlarmOn = false;
    Threads[i]->DeadlineAlarmOn = false;
    Threads[i]->RuntimeMonitoringOn = false;
  }
}

void OSBos::RunKernel(){
  unsigned long now = millis();
  Thread* t;

  for(int i=0; i<_numberOfThreads; i++){
    t = Threads[i]; 

    if(t->Active){
      if((unsigned long)(now - t->MillisLastExecuted) >= t->ReadyPeriod_ms){
        //now = millis();

        if(t->DeadlineAlarmOn && (now - t->MillisLastExecuted) > t->DeadlinePeriod_ms){
          if(t->DeadlineAlarm == 0){
            Serial.print("\nOSBos: Deadline alarm triggered, but not handled for task ");
            Serial.print(i);
            Serial.println(". This message is super inefficient. Set a flag.");
          }
          else{
            t->DeadlineAlarm();
          }
        }
        t->MillisLastExecuted = now;

        unsigned long preRun_us = 0;
        if(t->RuntimeMonitoringOn) preRun_us = micros();

        if(t->RootMethod == 0) Serial.println("\nOSBos: Thread not assigned.");
        else t->RootMethod();

        if(t->RuntimeMonitoringOn) t->LastRuntime_us = micros() - preRun_us;

        if(t->RuntimeAlarmOn && t->LastRuntime_us > t->RuntimeAlarm_us){
          if(t->RuntimeAlarm == 0){
            Serial.print("\nOSBos: Runtime alarm triggered, not handled for task");
            Serial.print(i);
            Serial.println(". This message is super inefficient. Set a flag.");
          } 
          else t->RuntimeAlarm();
        }
      }
    }
  }
}