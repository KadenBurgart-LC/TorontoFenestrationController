# Fenestration Controller Data Logger

A Python application that continuously polls the fenestration controller's web server to retrieve and log sensor data locally.

## Purpose

This data logger provides redundancy for the sensor data logging and eliminates the need to physically remove the SD card from the P1AM-200 controller to access data. Instead, data is automatically collected and stored on the local machine.

## Features

- Polls the `/wLastLogEntry` endpoint approximately once per second
- Saves only distinct entries to avoid duplicate data
- Organizes log files by date (YYYY-MM-DD.txt format)
- Displays entries in the console with color coding (green for data, red for errors)
- Handles network errors gracefully without crashing
- Automatically creates new log files when the date changes

## Installation

1. Ensure Python 3.6 or higher is installed
2. Install dependencies:
   ```
   pip install -r requirements.txt
   ```

## Usage

Run the data logger:
```
python data_logger.py
```

The application will run continuously until stopped with Ctrl+C.

## Log File Format

Log files are stored in the `logs` subfolder with filenames in the format `YYYY-MM-DD.txt`.

Each line in the log file contains comma-separated sensor data in the format:
```
timestamp,datetime,sensor1,sensor2,...
```

Example:
```
17707436480,2026-02-10 17:14:08,0.0,7.2,7,7,5.73,5.01,7.16,7.16,0.0,7.2,0.0,0,0,0,0,0,_
```

## Configuration

You can modify the following variables at the top of `data_logger.py`:

- `ENDPOINT_URL`: The web server endpoint to poll (default: http://192.168.1.178/wLastLogEntry)
- `POLL_INTERVAL`: Time between polls in seconds (default: 1.0)

## Running on Windows

To run continuously in the background on Windows:

1. **Run in a terminal window:**
   ```
   python data_logger.py
   ```

2. **Run as a scheduled task:**
   - Create a batch file to run the script
   - Use Windows Task Scheduler to run at system startup

3. **Run with a process manager:**
   - Consider using a tool like NSSM (Non-Sucking Service Manager) to run as a Windows service
