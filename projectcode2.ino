

import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO
from datetime import datetime
import time


BROKER = "localhost"            # Local Mosquitto broker (running on Pi)
TOPIC = "earthquake/magnitude"  # Topic subscribed to
BUZZER_PIN = 17                 # GPIO17 (Pin 11)
P_WAVE_THRESHOLD = 1.10         # Mild tremor threshold
S_WAVE_THRESHOLD = 1.40         # Strong earthquake threshold


GPIO.setmode(GPIO.BCM)
GPIO.setup(BUZZER_PIN, GPIO.OUT)
GPIO.output(BUZZER_PIN, GPIO.LOW)

def buzzer_beep(pattern="P"):
    """Trigger buzzer sound pattern for alert."""
    if pattern == "S":
        for _ in range(5):
            GPIO.output(BUZZER_PIN, GPIO.HIGH)
            time.sleep(0.3)
            GPIO.output(BUZZER_PIN, GPIO.LOW)
            time.sleep(0.2)
    elif pattern == "P":
        for _ in range(2):
            GPIO.output(BUZZER_PIN, GPIO.HIGH)
            time.sleep(0.15)
            GPIO.output(BUZZER_PIN, GPIO.LOW)
            time.sleep(0.15)
    else:
        GPIO.output(BUZZER_PIN, GPIO.LOW)

def classify_wave(magnitude, vibration):
    """Classify vibration type."""
    if magnitude > S_WAVE_THRESHOLD or vibration == 1:
        return "S-WAVE"
    elif magnitude > P_WAVE_THRESHOLD:
        return "P-WAVE"
    else:
        return "NORMAL"

def log_event(msg):
    """Save earthquake data with timestamp."""
    with open("/home/pi/earthquake_log.txt", "a") as f:
        f.write(f"{datetime.now().strftime('%Y-%m-%d %H:%M:%S')} - {msg}\n")

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print(" Connected to MQTT Broker!")
        client.subscribe(TOPIC)
        print(f"📡 Subscribed to topic: {TOPIC}")
    else:
        print("Failed to connect, return code:", rc)

def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode("utf-8").strip()
        if ',' not in payload:
            print("Invalid data received:", payload)
            return

        magnitude_str, vibration_str = payload.split(',')
        magnitude = float(magnitude_str)
        vibration = int(vibration_str)

        wave = classify_wave(magnitude, vibration)
        print(f"\n📨 Data: Magnitude={magnitude:.2f}, Vibration={vibration} → {wave}")

        if wave == "S-WAVE":
            print(" STRONG EARTHQUAKE DETECTED!")
            buzzer_beep("S")
            log_event(f"S-WAVE ALERT | Magnitude: {magnitude:.2f}")

        elif wave == "P-WAVE":
            print(" P-Wave Detected — minor vibration.")
            buzzer_beep("P")
            log_event(f"P-WAVE | Magnitude: {magnitude:.2f}")

        else:
            print("Normal condition.")
            GPIO.output(BUZZER_PIN, GPIO.LOW)

    except Exception as e:
        print("Error processing message:", e)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

print("\n Starting Earthquake Listener — press Ctrl+C to stop\n")

try:
    client.connect(BROKER, 1883, 60)
    client.loop_forever()

except KeyboardInterrupt:
    print("\n Exiting — cleaning up GPIO.")
    GPIO.cleanup()

except Exception as e:
    print("MQTT Connection failed:", e)
    GPIO.cleanup()
