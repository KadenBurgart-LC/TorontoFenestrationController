# Toronto Fenestration Controller Code
This application runs an industrial machine for testing windows and doors, and provides a user interface via an interactive Web Application.

The window testing wall simulates wind and rain conditions to evaluate windows for certification in North America. Windows are mounted against the wall, and the cavity between the window and wall is pressurized to various levels, and measured in various ways to qualify a particular assembly from the window product line for certification.

### Folders in this repository:
* **UI_Webpage:** The web app that acts as the user interface. This web app sends HTTP messages back and forth with a Web Server hosted on the embedded control system in the Fenestration Wall to control it, and read data from the sensors. There is HTML, CSS, and Javascript in here, as well as several assets (mostly icons right now). This folder contains its own README.md.
* **fenestrationController:** An Arduino application, written in C++, which controls the electromechanical systems on the Fenestration Wall, including the relay controls, PID loops, sensor signal processing, and data logging. The application also hosts a Web Server that allows users to control the machine through HTTP requests, indended to be made primarilly through the WebApp (UI_Webpage). The application runs a multithreading architecture that allows it to perform its tasks asynchronously. This folder contains its own README.md.
* **dataLogger:** TODO. A simple python app that requests data logger lines from the Web Server over HTTP, and stores them on the local machine. This provides redundancy in the data logging, and also makes it so that the user doesn't have to remove the SD card from the P1AM-200 to get the data; they can have it collect on their local machine instead.

The web application is built up using a bunch of "widgets" which communicate back and forth with a web server running on the embedded control system. The web server has tools for communicating with these widgets asynchronously, while juggling its other tasks as an industrial controller.

Various sensor signals are continuously logged by the controller, storing timestamped data on an SD card in the P1AM-200 CPU.