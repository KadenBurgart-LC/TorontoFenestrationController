#ifndef LIB_UTILS_H
#define LIB_UTILS_H

#include <Arduino.h>
#include <stdexcept>
#include <cstddef>
#include <cctype>
#include <stdlib.h>
#include <malloc.h>

extern "C"  char __end__; // Defined by linker: end of static variables / start of heap
extern "C" char *sbrk(int i); // Standard library function to check heap boundary

/* External symbols from linker script for stack boundaries */
extern  "C" uint32_t __StackTop;   // Top of stack (highest address)

// We don't have a stack limit on this microcontroller. The stack will grow right up on to the heap.
//extern  "C" uint32_t __StackLimit;   // Bottom of stack (lowest address, varies by board config)

namespace lib_Util {

    // MEMORY functions for the P1AM-200
    // On this chip, the stack starts at the top (higher address) of RAM and grows down.
    // The heap starts at the bottom of RAM and grows up.
    #define MAX_USEABLE_RAM 262144  // How much RAM (static stuff+heap+stack) are we allowed to use on this microcontroller?
    #define STACK_WATERMARK_BYTE 0xAA // 10101010 in binary

    inline void PrintRamAddresses(){
        Serial.print("Stack Top: "); Serial.println((uint32_t)&__StackTop, HEX);
        Serial.print("Heap Start: "); Serial.println((uint32_t)&__end__, HEX);
    }

    inline size_t TotalRamSize(){
        return (size_t)( (uint32_t)&__StackTop -  (uint32_t)&__end__);
    }

    /* At the start of the application, fill in the stack with STACK_WATERMARK_BYTE.
       Later, we can scan through memory to see how deep the stack has gotten, and if
       our program is at risk of overflow. */
    inline void WatermarkStackRam(){
        uint32_t current_sp;
        __asm__ volatile ("mov %0, sp" : "=r" (current_sp));

        // Fill from just below current SP, down to the stack limit
        uint8_t *stack_start = (uint8_t *)current_sp - 64; // Leave a small safety buffer from where we are now
        uint8_t *heap_top = (uint8_t *)sbrk(0) + 64; // Start watermarking some distance up from the top of the heap

        for (uint8_t *p = stack_start; p > heap_top; p--) *p = STACK_WATERMARK_BYTE;
    }

    /* Check to see how much stack RAM no longer has our watermark in it, so we know the total amount
       of stack size ever used since the start of the program. */
    inline size_t CheckRamSizeTraversed(){
        uint32_t current_sp;
        __asm__ volatile ("mov %0, sp" : "=r" (current_sp));

        uint8_t *crawl_start = (uint8_t *)sbrk(0) + 64; // Start crawling some distance up from the top of the heap
        uint8_t *crawler;

        for (crawler = crawl_start; (uint32_t)crawler < current_sp; crawler++) if (*crawler != STACK_WATERMARK_BYTE) break;

        return (size_t)( (uint32_t)&__StackTop - (uint32_t)crawler );
    }

    /* Check the stack pointer to see how much stack RAM we are currently using */
    inline size_t StackSizeNow(){
        uint32_t current_sp;
        __asm__ volatile ("mov %0, sp" : "=r" (current_sp));
        
        return (size_t)((uint32_t)&__StackTop - current_sp);
    }

    /* Check how much RAM is free right now */
    inline size_t FreeRamNow(){
        uint32_t current_sp;
        __asm__ volatile ("mov %0, sp" : "=r" (current_sp));
        uint32_t heap_top_adr = (uint32_t)sbrk(0);

        return (size_t)(current_sp - heap_top_adr);
    }

    /* How much heap RAM are we currently using? */
    inline size_t HeapSize(){
        char *heap_top = sbrk(0);

        return (size_t)(heap_top - &__end__);
    }

    /* Report the current RAM status on the Serial line */
    inline void PrintRamStatusToSerial(){
        size_t totalRamSz = TotalRamSize();
        size_t stackRamUsed = StackSizeNow();
        size_t heapRamUsed = HeapSize();
        size_t totalRamUsd = stackRamUsed + heapRamUsed;
        
        float stackPercentUsed = (float)stackRamUsed * 100.0f / (float)totalRamSz;
        float heapPercentUsed = (float)heapRamUsed * 100.0f / (float)totalRamSz;
        float totalPercentUsed = (float)(totalRamUsd) * 100.0f / (float)totalRamSz;

        Serial.print(F("\nCurrent stack RAM used: "));
        Serial.print(stackRamUsed);
        Serial.print(F("\t("));
        Serial.print(stackPercentUsed);
        Serial.println(F("% of total RAM)"));

        Serial.print(F("Current heap RAM used: "));
        Serial.print(heapRamUsed);
        Serial.print(F("\t("));
        Serial.print(heapPercentUsed);
        Serial.println(F("% of total RAM)"));

        Serial.print(F("Current total RAM used: "));
        Serial.print(totalRamUsd);
        Serial.print(F("\t("));
        Serial.print(totalPercentUsed);
        Serial.println(F("% of total RAM)\n"));
    }

    /* Report the RAM useage over the lifetime of the app, checking the watermark, to the Serial line */
    inline void PrintRamStackUseageToSerial(){
        size_t stackRamTraversed = CheckRamSizeTraversed();
        size_t totlRamSz = TotalRamSize();
        float stackPercentUsed = (float)stackRamTraversed * 100.0f / (float)totlRamSz;

        Serial.print(F("\nMax stack size since bootup: "));
        Serial.print(stackRamTraversed);
        Serial.print(F("\t("));
        Serial.print(stackPercentUsed);
        Serial.println(F("% of total RAM)"));
    }

	// Safe conversion of strings to floats
	inline float StringToFloat(const char* str, bool& success){
		success = false;
        
        char* end_ptr; // C-style pointer to track where parsing stops
        errno = 0; // Reset the global error code before calling strtof
        
        // This stops parsing at the first non-float character (like std::stof)
        float result = std::strtof(str, &end_ptr);
        
        
        // Check for NO NUMBER PARSED (e.g., "hello" or empty string)
        if (str == end_ptr) { 
            success = false;
            return 0.0f;
        }
        
        // Check for OUT OF RANGE (Overflow/Underflow)
        if (errno == ERANGE) {
            success = false;
            return 0.0f;
        }

        // C. Check for TRAILING JUNK (end_ptr != \0)
        bool justWhiteSpace = true;
        
        // Iterate over the remainder of the string starting from where strtof stopped.
        for (char* p = end_ptr; *p != '\0'; ++p) { 
            if (!std::isspace(static_cast<unsigned char>(*p))) {
                justWhiteSpace = false;
                break;
            }
        }

        if (justWhiteSpace) {
            success = true;
        } else {
            success = false;
        }
        
        if (success) return result;
        
        return 0.0f;
	}

	inline float StringToFloat(String str, bool& success){
		return StringToFloat(str.c_str(), success);
	}
}

#endif