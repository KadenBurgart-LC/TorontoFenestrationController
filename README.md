# Toronto Fenestration Controller Code
This repository contains the code to run a P1AM-200 controller from AutomationDirect.com, which is the 'main brain' in the electrical control cabinet for an NAFS window testing wall.

The window testing wall simulates wind and rain conditions to evaluate windows for certification in North America. Windows are mounted against the wall, and the cavity between the window and wall is pressurized to various levels, and measured in various ways to qualify a particular assembly from the window product line for certification.

Folders in this repository:
-Backups: Old stuff that I was afraid to delete but is no longer relevant. Now that the project is running on Git, this can probably be ignored forever.
-ESPserver (example): Once upon a time I made a control system for a fenestration wall using an ESP32 as the main brain. I copied that code over here so I could use it as a starting point. It can probably be ignored forever now, as the code has moved past the point that those old references are still helpful.
-UI_Webpage: The web app that acts as the user interface. There is HTML, CSS, and Javascript in here, as well as several assets (mostly icons right now).
