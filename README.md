# Toronto Fenestration Controller Code
This application runs an industrial machine for testing windows and doors, and provides a user interface via an interactive Web Application.

The window testing wall simulates wind and rain conditions to evaluate windows for certification in North America. Windows are mounted against the wall, and the cavity between the window and wall is pressurized to various levels, and measured in various ways to qualify a particular assembly from the window product line for certification.

Folders in this repository:
* UI_Webpage: The web app that acts as the user interface. There is HTML, CSS, and Javascript in here, as well as several assets (mostly icons right now).
* fenestrationController: The Arduino application that runs on the P1AM-200 controller from AutomationDirect.com.

The web application is built up using a bunch of "widgets" which communicate back and forth with a web server running on the embedded control system. The web server has tools for communicating with these widgets asynchronously, while juggling its other tasks as an industrial controller.

Various sensor signals are continuously logged by the controller, storing timestamped data on an SD card in the P1AM-200 CPU.