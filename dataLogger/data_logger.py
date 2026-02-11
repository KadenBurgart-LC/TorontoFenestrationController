import requests
import time
from datetime import datetime
from pathlib import Path
import os
from colorama import init, Fore, Style

# Initialize colorama for Windows color support
init(autoreset=True)

# Configuration
LOGS_DIR = Path(__file__).parent / "logs"
SETTINGS_FILE = Path(__file__).parent / "settings.txt"

def load_settings():
    """Load settings from settings.txt file."""
    settings = {
        'ENDPOINT_URL': 'http://192.168.1.178/wLastLogEntry',
        'POLL_INTERVAL': 1.0
    }

    if SETTINGS_FILE.exists():
        with open(SETTINGS_FILE, 'r') as f:
            for line in f:
                line = line.strip()
                # Skip comments and empty lines
                if line and not line.startswith('#'):
                    if '=' in line:
                        key, value = line.split('=', 1)
                        key = key.strip()
                        value = value.strip()

                        if key == 'ENDPOINT_URL':
                            settings['ENDPOINT_URL'] = value
                        elif key == 'POLL_INTERVAL':
                            try:
                                settings['POLL_INTERVAL'] = float(value)
                            except ValueError:
                                print(f"{Fore.RED}WARNING: Invalid POLL_INTERVAL value, using default")

    return settings

def ensure_logs_directory():
    """Create logs directory if it doesn't exist."""
    LOGS_DIR.mkdir(exist_ok=True)

def get_log_filename():
    """Generate log filename based on current date (YYYY-MM-DD.txt)."""
    return f"{datetime.now().strftime('%Y-%m-%d')}.txt"

def get_last_timestamp(log_file_path):
    """Read the last timestamp from the log file."""
    if not log_file_path.exists():
        return None

    with open(log_file_path, 'r') as f:
        lines = f.readlines()
        if lines:
            last_line = lines[-1].strip()
            # Extract timestamp (first field before comma)
            timestamp = last_line.split(',')[0] if ',' in last_line else None
            return timestamp
    return None

def append_to_log(log_file_path, entry):
    """Append a new entry to the log file."""
    with open(log_file_path, 'a') as f:
        f.write(entry + '\n')

def poll_endpoint(endpoint_url):
    """Poll the endpoint and return the response text."""
    try:
        response = requests.get(endpoint_url, timeout=5)
        response.raise_for_status()
        return response.text.strip()
    except requests.exceptions.RequestException as e:
        return None, str(e)

def main():
    settings = load_settings()
    ENDPOINT_URL = settings['ENDPOINT_URL']
    POLL_INTERVAL = settings['POLL_INTERVAL']

    print("Fenestration Controller Data Logger")
    print(f"Polling endpoint: {ENDPOINT_URL}")
    print(f"Log directory: {LOGS_DIR}")
    print(f"Poll interval: {POLL_INTERVAL} seconds")
    print("-" * 60)

    ensure_logs_directory()

    last_timestamp = None
    current_log_file = None

    while True:
        try:
            # Check if we need to switch to a new log file (new day)
            log_filename = get_log_filename()
            log_file_path = LOGS_DIR / log_filename

            # If the log file changed (new day), reload last timestamp
            if current_log_file != log_filename:
                current_log_file = log_filename
                last_timestamp = get_last_timestamp(log_file_path)
                print(f"\n{Fore.GREEN}Logging to: {log_filename}")

            # Poll the endpoint
            result = poll_endpoint(ENDPOINT_URL)

            if isinstance(result, tuple):
                # Error occurred
                _, error_msg = result
                print(f"{Fore.RED}ERROR: Unable to reach endpoint - {error_msg}")
            else:
                entry = result

                if entry:
                    # Extract timestamp from entry (first field)
                    current_timestamp = entry.split(',')[0] if ',' in entry else entry

                    # Check if this entry has a new timestamp
                    if current_timestamp != last_timestamp:
                        # Log to file
                        append_to_log(log_file_path, entry)
                        last_timestamp = current_timestamp

                        # Display to console
                        print(f"{Fore.GREEN}{entry}")

            # Wait before next poll
            time.sleep(POLL_INTERVAL)

        except KeyboardInterrupt:
            print("\n\nData logger stopped by user.")
            break
        except Exception as e:
            print(f"{Fore.RED}ERROR: Unexpected error - {str(e)}")
            time.sleep(POLL_INTERVAL)

if __name__ == "__main__":
    main()
