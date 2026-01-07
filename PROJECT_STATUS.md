# Fenestration Controller - Project Status

**Last Updated:** 2026-01-05

## System Overview

Industrial window/door testing machine controller with web-based UI. Tests windows against wind and rain conditions for North American certification.

### Hardware Stack
- **Controller:** AutomationDirect P1AM-200 (Arduino-compatible)
- **I/O Modules:**
  - Slot 1: P1-15TD1 (16-CH Digital Output)
  - Slot 2: P1-08ADL-1 (8-CH Analog Current Input)
  - Slot 3: P1-08ADL-2 (8-CH Analog Voltage Input)
- **Network:** MKR Ethernet Shield, Static IP 192.168.1.177
- **Storage:** SD card for data logging
- **RTC:** PCF8563 for timestamps

### Software Stack
- **Embedded:** Arduino C++ with OSBos cooperative multithreading kernel
- **Web Server:** Custom HTTP server on P1AM-200
- **Web UI:** HTML/CSS/JavaScript (jQuery + D3.js v4)
- **Architecture Pattern:** Widget-based request/response system

### Physical System (Incomplete - Not Built Yet)
- **High Pressure System:** Structural testing, various pressures
- **Low Pressure System:** Air leakage testing (±75Pa target)
- **Primary Controls:** Proportional valves for pressure control
- **Sensors:** Multiple pressure ranges, displacement, temperature, humidity
- **Actuators:** Blowers, water pump, control valves

## Current Development Status

### ✅ COMPLETED / WORKING

#### Core Infrastructure
- [x] OSBos kernel integration with cooperative multithreading
- [x] HAL abstraction layer framework (needs sensor calibration)
- [x] Web server with HTTP request routing
- [x] Widget handler system architecture
- [x] Serial console interface (SerialConsole library)
- [x] SD card file operations (read/write/append/delete)
- [x] RTC integration with timestamp functions
- [x] Data logger with rolling buffer (20 entries)
- [x] Data logger SD card persistence (daily CSV files)

#### Web Server Endpoints (Backend Complete)
- [x] `/` - Index route
- [x] `/liveDataPacketRequest` - Batch live data JSON endpoint
- [x] `/wSTOP_ALL` - Emergency stop button
- [x] `/wRTC` - Real-time clock display
- [x] `/wTargetPressure` - Target pressure value sender
- [x] `/wLowPressure` - Low pressure sensor live value
- [x] `/wMedPressure` - Medium pressure sensor live value
- [x] `/wHighPressure` - High pressure sensor live value
- [x] `/wDisplacement1` - Displacement sensor 1 live value
- [x] `/wDisplacement2` - Displacement sensor 2 live value
- [x] `/wLastLogEntry` - Last log line display
- [x] `/wWaterPump` - Water pump toggle
- [x] `/wHPvalvesDirection` - High pressure valve direction (stub/example)
- [x] `/wLPvalvesDirection` - Low pressure valve direction toggle
- [x] Example widget handlers (all types demonstrated)

#### Web UI (Frontend Complete)
- [x] Main grid layout with responsive design
- [x] Widget CSS styling for all widget types
- [x] Live data subscription system (1Hz polling)
- [x] Widget implementations:
  - [x] Button widgets
  - [x] Toggle widgets (with error states)
  - [x] Value sender widgets
  - [x] Live short value widgets (with go/pause)
  - [x] Long live value widgets
  - [x] Smart short value widgets
  - [x] Smart label value widgets
  - [x] Alarm widgets
  - [x] Label value widgets (static)
- [x] D3.js graph framework (needs endpoint updates)
- [x] RTC auto-refresh (30s interval)

#### Mechanical System
- [x] Target pressure storage and limits
- [x] Low pressure valve configuration tracking
- [x] High pressure valve configuration tracking
- [x] tk_StopAll task (example async task)
- [x] tk_SetLowPressure_Positive task
- [x] tk_SetLowPressure_Negative task

### 🚧 IN PROGRESS / PARTIALLY COMPLETE

#### Hardware Abstraction Layer
- [ ] **Analog sensor calibration values** - All sensors use placeholder slot/channel mappings
  - All pressure sensors currently map to slot 2, channel 1
  - Need real gain/offset values for each sensor type
  - Need correct P1 slot/channel assignments
- [ ] **Duplicate definition bug** - `PRESSURE_LFE_DIFFERENTIAL` defined twice (HAL.cpp:101-115)
- [ ] **Missing sensor definitions** - TEMP_AMB, TEMP_LFE, HUMIDITY_AMB in enum but not in map
- [ ] **Digital output mappings** - Only 3 outputs mapped (structural blower, leakage blower, water pump)

#### Web UI - Backend Gaps
- [ ] **User Interface** - The UI/UX design for widget layout isn't really done, and is currently rather shit
- [ ] `/wTmpAmb` - Ambient temperature widget (frontend exists, no backend)
- [ ] `/wHpBlower` - High pressure blower toggle (frontend exists, no backend)
- [ ] `/wLpBlower` - Low pressure blower toggle (frontend exists, no backend)
- [ ] `/wHPvalvesDirection` - Currently uses example stub, needs real implementation

#### Graph System
- [ ] Graph endpoint `/get/pressure` doesn't exist (graph.js:130)
- [ ] Graph data updates not hooked to real data sources
- [ ] Graph currently has hardcoded test data transformations

### ❌ NOT STARTED / TODO

#### Critical Path - PID Control System
- [ ] **Timer interrupt setup** - Hardware timer configuration for consistent PID execution
- [ ] **PID library integration or custom implementation**
- [ ] **High pressure PID loop** - Control valve to reach target pressure
- [ ] **Low pressure PID loop** - Control valve for ±75Pa range
- [ ] **PID tuning interface** - Web widgets for Kp, Ki, Kd adjustment
- [ ] **Anti-windup implementation**
- [ ] **Valve output limiting and safety**

#### Mechanical System Tasks
- [ ] `tk_SetHighPressure_Positive` - Not implemented
- [ ] `tk_SetHighPressure_Negative` - Not implemented
- [ ] `th_HighPressurePID_RUN` - Not implemented
- [ ] `th_LowPressurePID_RUN` - Not implemented
- [ ] Valve control abstraction (proportional valves)
- [ ] Blower control logic
- [ ] Safety interlocks and limit checking

#### Data Logger Enhancements
- [ ] Logger on/off control (with safety checks)
- [ ] Logger status indicator
- [ ] Error state tracking and reporting
- [ ] Web endpoint to serve archived log files

#### Testing Infrastructure
- [ ] Automated test scripts (currently test via web UI manually)
- [ ] Unit tests for HAL functions
- [ ] Widget communication tests
- [ ] PID simulation for testing without hardware
- [ ] Mock sensor data generation

#### Python Data Logger Client
- [ ] HTTP polling script
- [ ] Local file storage
- [ ] Auto-reconnection logic
- [ ] Error handling and logging

#### Future Enhancements
- [ ] Machine simulator for development without hardware
- [ ] DHCP support with mDNS/Bonjour for easier network setup
- [ ] Web server hosts its own UI (currently must download separately)
- [ ] Alarms and safety limit monitoring
- [ ] Test sequence automation
- [ ] Historical data graphing
- [ ] Export test reports (CSV, PDF)

## Known Issues & Bugs

### High Priority
1. **HAL.cpp:101-115** - Duplicate `PRESSURE_LFE_DIFFERENTIAL` definition, second should be `PRESSURE_LFE_ABSOLUTE`
2. **HAL.cpp:40-145** - All analog sensor definitions use placeholder P1 slot/channel values (all set to 2,1)
3. **graph.js:130** - References non-existent endpoint `/get/pressure`
4. **Incomplete widget backend** - wTmpAmb, wHpBlower, wLpBlower have frontend but no backend handlers

### Medium Priority
5. **Static IP hardcoded** - No easy network configuration (th_WebServer.cpp:21)
6. **Missing sensor map entries** - TEMP_AMB, TEMP_LFE, HUMIDITY_AMB, PRESSURE_LFE_ABSOLUTE not in AnalogSignals map
7. **Example code in production** - th_test.h, wExample_* widgets still present

### Low Priority
8. **RTC library frustration** - Comment in HAL.cpp:379 suggests RTC library issues
9. **Code comments have typos** - "spinns" instead of "spins", etc.

## Development Priorities

### Phase 1: Infrastructure Cleanup (Current)
1. Fix HAL duplicate definition bug
2. Map missing widgets to backend handlers
3. Document all required sensors with specs
4. Create sensor calibration tracking document

### Phase 2: PID Foundation (Next Critical Step)
1. Research and select timer interrupt approach for SAMD
2. Set up hardware timer ISR for PID execution (10-100Hz target)
3. Implement or integrate PID controller library
4. Create PID tuning web interface widgets

### Phase 3: Hardware Integration (Blocked - Waiting for Physical Build)
1. Install and wire all sensors
2. Calibrate analog inputs with real hardware
3. Map digital outputs to physical relays
4. Test valve control and feedback

### Phase 4: Control System Implementation
1. Implement high pressure PID control loop
2. Implement low pressure PID control loop
3. Add safety interlocks and limits
4. Tune PID parameters for system response

### Phase 5: Testing & Refinement
1. Create automated test suite
2. Build software machine simulator
3. Stress test web server under load
4. Document operational procedures

## Architecture Patterns to Follow

### Widget Backend Pattern
```cpp
// In GetWidgetStrVal():
else if (strcmp(key, "wMyWidget") == 0){
    success = true;
    return String(HAL::getAnalogInputFloat(HAL::AnalogInput::MY_SENSOR));
}

// In SetWidgetVal():
else if(strcmp(key, "wMyWidget") == 0) {
    // conversion and validation
    success = HAL::setDigitalOutput(HAL::DigitalOutput::MY_OUTPUT, value);
}

// In th_WebServer::initialize():
Jarvis.On("/wMyWidget", routes::WidgetHandler_Toggle("wMyWidget"));
```

### HAL Sensor Definition Pattern
```cpp
{HAL::AnalogInput::MY_SENSOR, ([]() {
    HAL::AnalogSignalDefinition def;
    def.MeasurementType = HAL::AnalogIntermediateMeasurementType::CURRENT;
    def.IntermediateMeasurementType_ConversionDenominator = 409.55;
    def.SignalUnits = "Pa";
    def.SignalUnitGain = 1.0;    // Sensor-specific calibration
    def.SignalUnitOffset = 0.0;  // Sensor-specific calibration
    def.P1_Slot = 2;
    def.P1_Channel = 1;
    return def;
})()}
```

### OSBos Task Pattern
```cpp
namespace tk_MyTask {
    Thread Task = NewTerminalTask(Tick, 200);

    int8_t Tick(){
        static uint8_t state = 0;

        if (state == 0){
            // Initialize
            state++;
        }
        else if (state == 1){
            // Do work
            state++;
        }
        else if (state == 2){
            // Cleanup
            state = 0;
            return 1; // Task complete
        }

        return 0; // Keep running
    }
}
```

## Questions & Decisions Needed

### Hardware Specifications
- [ ] What are the exact sensor models and calibration specs?
- [ ] What proportional valve models are being used?
- [ ] Feedback mechanism for valve position?
- [ ] Blower specifications (on/off or variable speed)?

### Control System Design
- [ ] Desired PID execution rate? (10Hz, 50Hz, 100Hz?)
- [ ] Target response time for pressure control?
- [ ] Acceptable overshoot percentage?
- [ ] Safety pressure limits (absolute max/min)?

### Testing Approach
- [ ] Build software simulator or wait for physical machine?
- [ ] Create test fixture with single valve + sensor?
- [ ] Virtual testing strategy?

## Files Structure Reference

```
FenestrationController/
├── README.md
├── PROJECT_STATUS.md (this file)
├── fenestrationController/
│   ├── README.md
│   ├── fenestrationController.ino       # Main entry point
│   ├── HAL.h / HAL.cpp                  # Hardware abstraction
│   ├── MechanicalSystem.h / .cpp        # Business logic
│   ├── th_WebServer.h / .cpp            # HTTP server
│   ├── th_DataLogger.h / .cpp           # Data logging
│   ├── th_SerialConsole.h / .cpp        # USB serial interface
│   ├── th_Blink.h / .cpp                # LED blink example
│   ├── th_test.h                        # Test task example
│   ├── lib_Eth.h                        # Ethernet utilities
│   └── lib_Utils.h                      # General utilities
├── UI_Webpage/
│   ├── README.md
│   ├── index.html                       # Main UI page
│   ├── style/style.css                  # Widget styling
│   ├── scripts/
│   │   ├── main.js                      # Core UI logic
│   │   ├── graph.js                     # D3.js graphing
│   │   ├── widgets_common.js            # Shared widget code
│   │   ├── widgets_toggle.js
│   │   ├── widgets_button.js
│   │   ├── widgets_live-short-value.js
│   │   ├── widgets_long-live-value.js
│   │   ├── widgets_smart-short-value.js
│   │   ├── widgets_smart-label-value.js
│   │   ├── widgets_value-sender.js
│   │   └── widgets_alarm.js
│   └── assets/                          # Icons and images
└── dataLogger/                          # TODO: Python client app
```

## Library Dependencies

### Arduino Libraries (Installed via IDE)
- **P1AM** - P1AM-200 hardware driver (https://github.com/facts-engineering/P1AM)
- **OSBos** - Cooperative multithreading kernel (https://github.com/actuvon/OSBos)
- **SerialConsole** - CLI interface library (https://github.com/actuvon/SerialConsole)
- **ArduinoJson** v7.4.2 - JSON parsing (Benoit Blanchon)
- **Adafruit_NeoPixel** - RGB LED control (https://github.com/adafruit/Adafruit_NeoPixel)
- **PCF8563** - RTC library (https://github.com/facts-engineering/PCF8563_RTC)
- **TimeLib** - Time utilities (https://github.com/PaulStoffregen/Time)
- **StreamUtils** - Stream helpers (https://github.com/bblanchon/ArduinoStreamUtils)
- **SD** - SD card operations (standard Arduino)
- **Ethernet** - Ethernet shield (standard Arduino)

### Web UI Libraries (Included)
- **jQuery** v3.7.1 - DOM manipulation and AJAX
- **D3.js** v4 - Data visualization and graphing

## Notes for Claude (AI Assistant)

### Role Definition
- **Project Manager:** Track todos, priorities, and blockers
- **Pattern Replicator:** Replicate established code patterns when adding features
- **Code Organization Helper:** Keep architecture clean and consistent
- **Documentation Maintainer:** Keep this file and others up to date

### Working Guidelines
1. **Always read before writing** - Understand existing code before modifications
2. **Follow established patterns** - Don't reinvent, replicate working patterns
3. **Ask before major changes** - Architecture decisions need user approval
4. **Test manually via web UI** - No automated tests yet, verify through browser
5. **Update this file** - Keep PROJECT_STATUS.md current with changes

### Current Focus Areas
- Preparing for PID implementation (highest priority)
- Fixing identified bugs in HAL
- Completing widget backend handlers
- Organizing sensor specifications and calibration data

### Communication Style
- Concise, technical, direct
- No unnecessary praise or validation
- Focus on facts and problem-solving
- Question assumptions when something seems off
