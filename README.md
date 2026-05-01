# Accident Detection and Alert System

> Automatic crash detection with instant GPS-location SMS alert to emergency contacts

## How It Works
1. MPU6050 accelerometer continuously monitors G-force
2. Sudden impact > 2.5G triggers detection
3. 5-second confirmation window (prevents false alerts)
4. SIM800L sends SMS with Google Maps link to emergency contact

## Hardware
| Component | Purpose |
|-----------|---------|
| Arduino Uno | Main controller |
| MPU6050 | Accelerometer/Gyroscope |
| SIM800L GSM | Send SMS alerts |
| NEO-6M GPS | Get location coordinates |
| Buzzer + LED | Local alert indicators |

## Setup
1. Insert SIM card into SIM800L (with calling plan)
2. Set emergency number in ALERT_NUMBER
3. Adjust IMPACT_THRESHOLD as needed (2.5G default)
4. Upload code, place in vehicle

## Author
**Mothi Charan Naik Desavath** - Embedded Systems Engineer