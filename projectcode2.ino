import serial
import time
from datetime import datetime
from colorama import Fore, Style, init

# Initialize colored output
init(autoreset=True)

# === Serial Port Configuration ===
SERIAL_PORT = '/dev/ttyUSB0'   # Change if your Arduino shows different port
BAUD_RATE = 9600

# === Detection Thresholds ===
P_WAVE_THRESHOLD = 1.10   
S_WAVE_THRESHOLD = 1.40  

# === Setup Serial Communication ===
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for Arduino to initialize
    print(Fore.CYAN + f"Connected to Arduino on {SERIAL_PORT}\n")
except Exception as e:
    print(Fore.RED + f" Could not connect to Arduino: {e}")
    exit()

# === Helper Functions ===
def classify_wave(magnitude, vibration):
    """Determine type of wave based on thresholds and vibration sensor."""
    if magnitude > S_WAVE_THRESHOLD or vibration == 1:
        return "S-WAVE"
    elif magnitude > P_WAVE_THRESHOLD:
        return "P-WAVE"
    else:
        return "NORMAL"

def trigger_alarm():
    """Send alarm command to Arduino."""
    ser.write(b'A')
    print(Fore.MAGENTA + "🔔 ALERT Command Sent to Arduino (Buzzer ON)")

def log_event(event_type, magnitude):
    """Log detections with timestamp."""
    with open("earthquake_log.txt", "a") as f:
        f.write(f"{datetime.now()}, {event_type}, {magnitude:.2f}\n")

# === Main Loop ===
print(Fore.CYAN + "🌍 AI-Based Earthquake Detection System Started\n")
print(Fore.WHITE + "Listening for data from Arduino...\n")

try:
    while True:
        line = ser.readline().decode('utf-8').strip()

        # Skip if invalid
        if not line or ',' not in line:
            continue

        try:
            magnitude_str, vibration_str = line.split(',')
            magnitude = float(magnitude_str)
            vibration = int(vibration_str)

            wave_type = classify_wave(magnitude, vibration)

            if wave_type == "S-WAVE":
                print(Fore.RED + f" STRONG EARTHQUAKE DETECTED! Magnitude: {magnitude:.2f}")
                trigger_alarm()
                log_event("S-WAVE", magnitude)
                time.sleep(3)

            elif wave_type == "P-WAVE":
                print(Fore.YELLOW + f" P-Wave Detected (Minor vibration): {magnitude:.2f}")
                log_event("P-WAVE", magnitude)
                time.sleep(1)

            else:
                print(Fore.GREEN + f" Normal | Mag: {magnitude:.2f}")
                time.sleep(0.2)

        except ValueError:
            continue

except KeyboardInterrupt:
    print(Fore.CYAN + "\n System stopped by user.")
    ser.close()
