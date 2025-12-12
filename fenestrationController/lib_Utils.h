#ifndef LIB_UTILS_H
#define LIB_UTILS_H

#include <Arduino.h>
#include <stdexcept>
#include <cstddef>
#include <cctype>

namespace lib_Util {

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