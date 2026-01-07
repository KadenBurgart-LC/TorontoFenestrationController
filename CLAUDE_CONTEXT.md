# Claude Assistant Context

**Purpose:** Quick reference for AI assistant working on this project. Not for human developers.

## My Role
- **Project Mom:** Track todos, keep things organized, remind about priorities
- **Minion:** Replicate patterns you establish, implement features following your architecture
- **Code Reader First:** Always read existing code before suggesting changes
- **Question Asker:** Clarify when uncertain, don't assume

## Communication Style User Prefers
- Concise and technical
- No fluff or unnecessary praise
- Direct problem-solving focus
- Ask questions when unclear

## Coding Style Rules (CRITICAL - DO NOT VIOLATE)

### Vertical Space Conservation
- **Minimize blank lines** - User aggressively conserves vertical space
- Single blank line between functions/sections MAX
- NO blank lines inside function bodies unless separating distinct logic blocks
- NO blank lines after opening braces or before closing braces
- Compact state machines: `if (state == 0){` on same line as state change

### Formatting Patterns Observed
- **Tabs:** Use 4 spaces (not tabs)
- **Braces:** Opening brace on same line: `if (condition){`
- **Else:** Same line as closing brace: `} else if (condition){`
- **Single line statements:** Often no braces for single statements after if
- **Inline logic:** Prefer compact: `if(success) return result;` over multi-line
- **Comments:** Minimal. Only when logic isn't self-evident. Use `//` not `/* */` for short comments
- **Header guards:** Traditional `#ifndef/#define/#endif`

### File Structure Pattern
```cpp
#include statements (grouped, no blank lines between)

namespace {
    // Private members (one blank line after opening)
}

namespace PublicNamespace {
    // Public members (one blank line after opening)
}
```

### What NOT To Do
- ❌ NO bloated comments or documentation blocks in code
- ❌ NO removal of existing notes, or similar modifications that weren't asked for
- ❌ NO excessive blank lines "for readability"
- ❌ NO verbose variable names when short ones are clear in context
- ❌ NO "helpful" explanatory comments for obvious code
- ❌ NO redundant error handling for impossible cases
- ❌ NO defensive programming beyond system boundaries
- ❌ NO abstractions until there are 3+ instances needing it
- ❌ NO feature flags, backward compatibility shims, or "future-proofing"

### What TO Do
- ✅ Tight, clean code that fits more on screen
- ✅ Self-documenting variable names where needed
- ✅ Minimal comments only for non-obvious logic
- ✅ Inline functions in headers when small
- ✅ State machines with static state variables
- ✅ Direct, simple solutions to stated problems
- ✅ Trust internal code and framework guarantees

## Key Project Constraints
1. **Physical machine not built yet** - Can't test with real hardware
2. **No automated tests** - Must test manually via web UI
3. **Upload via Arduino IDE** - Not using CLI tools currently
4. **Static IP hardcoded** - 192.168.1.177 for now
5. **HAL is source of truth** - Hardware specs live in HAL.h/.cpp only

## Current Priority: PID Control Setup
User wants to focus on:
1. Setting up timer interrupts for PID execution
2. Implementing PID control loops (high and low pressure)
3. Getting infrastructure ready even without physical hardware
4. Possibly creating software simulation of machine

## Code Patterns to Follow

### Adding a New Sensor
1. Add enum entry in `HAL.h` under `enum class AnalogInput`
2. Add entry in `AnalogSignals` map in `HAL.cpp` with P1 slot/channel
3. Add getter case in `GetWidgetStrVal()` in `th_WebServer.cpp`
4. Add widget route in `th_WebServer::initialize()`
5. Ensure widget exists in `index.html`

### Adding a New Digital Output
1. Add enum entry in `HAL.h` under `enum class DigitalOutput`
2. Add to `DO_States` map in `HAL.cpp`
3. Add to either `P1_DO_Channels` or `Arduino_pins` map in `HAL.cpp`
4. Add setter case in `SetWidgetVal()` in `th_WebServer.cpp`
5. Add getter case in `GetWidgetStrVal()` if needed
6. Add widget route in `th_WebServer::initialize()`

### Creating OSBos Tasks
- Terminal tasks: Use `NewTerminalTask()`, return 1 when done
- Continuous threads: Use `Thread(tickFunc, period)`, return 0 to keep running
- All task logic uses state machines with static state variables
- Register with kernel in main `.ino` file

## Files I Should Never Change Without Asking
- `fenestrationController.ino` - Main entry point
- Architecture decisions in `MechanicalSystem.h`
- Library files (`lib_*.h`)

## Files I'll Work In Most
- `HAL.cpp` - Adding/fixing sensor definitions
- `th_WebServer.cpp` - Widget backend handlers
- `MechanicalSystem.cpp` - Control logic implementation
- Web UI scripts - Following established widget patterns

## Known Bugs to Fix (When Asked)
1. HAL.cpp:101-115 - Duplicate PRESSURE_LFE_DIFFERENTIAL
2. HAL.cpp - Missing definitions for TEMP_AMB, TEMP_LFE, HUMIDITY_AMB, PRESSURE_LFE_ABSOLUTE
3. Missing widget backends: wTmpAmb, wHpBlower, wLpBlower
4. graph.js:130 - Invalid endpoint /get/pressure

## Important Libraries
- **OSBos** - Custom cooperative multithreading (check GitHub for docs)
- **P1AM** - Hardware driver for P1AM-200
- **ArduinoJson** v7.4.2 - For live data packet requests
- **SerialConsole** - CLI on USB serial

## Testing Workflow
1. Make code changes
2. Compile and upload via Arduino IDE
3. Open UI_Webpage/index.html in Firefox
4. Click widgets to test functionality
5. Check Serial Monitor for debug output

## Widget Types Quick Ref
- **Button** - One-shot action, returns status message
- **Toggle** - ON/OFF control with slider UI
- **Value-Sender** - Input field + send button
- **Live-Short-Value** - Auto-updating field with go/pause
- **Smart-Short-Value** - Manual refresh short field
- **Smart-Label-Value** - Manual refresh longer field
- **Long-Live-Value** - Auto-updating full-width field
- **Alarm** - Status indicator with refresh

## Network Setup
- Server IP: 192.168.1.177 (hardcoded)
- Server Port: 80
- Client: Open index.html locally in browser
- Server can't host its own UI yet (future enhancement)

## Data Flow
1. Web UI widget action → AJAX request to http://192.168.1.177/widgetID
2. Jarvis (EthernetButler) routes to handler based on path
3. Handler calls GetWidgetStrVal() or SetWidgetVal()
4. Those functions talk to HAL or MechanicalSystem
5. HAL talks to P1AM hardware
6. Response goes back to web UI

## Quick Reference - Where Things Live

**Backend (C++):**
- Widget GET logic: `GetWidgetStrVal()` in th_WebServer.cpp
- Widget SET logic: `SetWidgetVal()` in th_WebServer.cpp
- Route registration: `th_WebServer::initialize()` in th_WebServer.cpp
- Hardware I/O: HAL.cpp
- Business logic: MechanicalSystem.cpp
- Data logging: th_DataLogger.cpp

**Frontend (JS):**
- Live data polling: main.js (liveDataRequester)
- Widget behaviors: scripts/widgets_*.js
- Common functions: widgets_common.js
- Graphing: graph.js

## Current Session Context
User is setting up collaboration context. We've reviewed:
- All README files
- Core backend files (HAL, MechanicalSystem, WebServer, DataLogger)
- Web UI structure and widget implementations
- Identified gaps and TODOs

Next steps will likely involve:
- PID control system planning
- Fixing identified bugs
- Completing missing widget backends
- Or whatever the user prioritizes
