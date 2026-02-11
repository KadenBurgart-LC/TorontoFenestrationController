/* H_INT.h

   Hardware Interrupt Configuration for P1AM-200

   This file configures hardware timer interrupts for the P1AM-200 controller (SAMD21 ARM Cortex-M0+).
   The interrupt is configured to trigger at 10Hz (100ms period) for PID control loops.

   Usage:
     1. Call HINT::initialize() in setup() before starting the kernel
     2. Attach your PID callback function using HINT::attachCallback(yourFunction)
     3. Enable the interrupt with HINT::enable()
     4. Disable when needed with HINT::disable()

   Example:
     void myPidLoop() {
       // Your PID control code here
       // This runs at 10Hz in an interrupt context
     }

     void setup() {
       HINT::initialize();
       HINT::attachCallback(myPidLoop);
       HINT::enable();
     }

   IMPORTANT: Keep interrupt handlers SHORT and FAST. No Serial.print(), no delays,
   no blocking operations. Only set flags and update variables.

   Hardware: Uses TC3 (Timer/Counter 3) on SAMD21 for minimal conflicts with Arduino core.
*/

#ifndef HINT_H
#define HINT_H

#include <Arduino.h>

namespace {
	// Private interrupt state and configuration

	// Function pointer for user callback
	void (*userCallback)() = nullptr;

	// Interrupt state flags
	volatile bool interruptEnabled = false;
	volatile uint32_t interruptCount = 0;

	// TC3 interrupt handler - called by hardware at 10Hz
	void TC3_Handler() {
		// Clear the interrupt flag
		TC3->COUNT16.INTFLAG.bit.MC0 = 1;

		// Increment counter for diagnostics
		interruptCount++;

		// Call user callback if attached
		if (userCallback != nullptr) {
			userCallback();
		}
	}
}

namespace HINT {

	/* Initialize the hardware timer interrupt
	   Call this once in setup() before enabling the interrupt */
	inline void initialize() {
		// Enable GCLK for TC3 (Generic Clock Generator 0 = 48MHz)
		GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN |
		                               GCLK_CLKCTRL_GEN_GCLK0 |
		                               GCLK_CLKCTRL_ID_TCC2_TC3);
		while (GCLK->STATUS.bit.SYNCBUSY);

		// Disable TC3 while configuring
		TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
		while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

		// Configure TC3 in 16-bit mode
		TC3->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 |  // 16-bit counter mode
		                         TC_CTRLA_WAVEGEN_MFRQ |   // Match frequency generation
		                         TC_CTRLA_PRESCALER_DIV1024; // Prescaler = 1024

		// Calculate compare value for 10Hz interrupt
		// CPU Clock = 48MHz, Prescaler = 1024, Target = 10Hz
		// Compare = (48MHz / 1024) / 10Hz = 4687.5 ≈ 4688
		TC3->COUNT16.CC[0].reg = 4688;
		while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

		// Enable MC0 interrupt (Match/Compare 0)
		TC3->COUNT16.INTENSET.reg = TC_INTENSET_MC0;

		// Enable TC3 interrupt in NVIC (Nested Vectored Interrupt Controller)
		NVIC_EnableIRQ(TC3_IRQn);
		NVIC_SetPriority(TC3_IRQn, 0);  // Highest priority
	}

	/* Attach a callback function to be called on each interrupt
	   The callback should be FAST and non-blocking */
	inline void attachCallback(void (*callback)()) {
		userCallback = callback;
	}

	/* Detach the callback function */
	inline void detachCallback() {
		userCallback = nullptr;
	}

	/* Enable the hardware interrupt - starts calling your callback at 10Hz */
	inline void enable() {
		if (!interruptEnabled) {
			interruptEnabled = true;
			TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
			while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
		}
	}

	/* Disable the hardware interrupt - stops calling your callback */
	inline void disable() {
		if (interruptEnabled) {
			interruptEnabled = false;
			TC3->COUNT16.CTRLA.reg &= ~TC_CTRLA_ENABLE;
			while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
		}
	}

	/* Check if the interrupt is currently enabled */
	inline bool isEnabled() {
		return interruptEnabled;
	}

	/* Get the total number of interrupts that have fired since initialization
	   Useful for diagnostics and timing verification */
	inline uint32_t getInterruptCount() {
		return interruptCount;
	}

	/* Reset the interrupt counter to zero */
	inline void resetInterruptCount() {
		noInterrupts();
		interruptCount = 0;
		interrupts();
	}
}

#endif