# Accident Detection & Alert System -- GPS + GSM SMS

> ESP32-based vehicle accident detection system using ADXL335 accelerometer for crash sensing, GPS module for location tracking, and SIM900 GSM module for instant SMS alerts with Google Maps link to emergency contacts.

## Overview

Built an ESP32-based vehicle accident detection and emergency alert system. The system continuously reads 3-axis acceleration data from an ADXL335 analog accelerometer, calculates the resultant G-force vector, and triggers an alert when the magnitude exceeds a configurable threshold (2.5g). On detection, the system fetches real-time GPS coordinates via a UART-connected GPS module (NEO-6M), displays the accident status and location on a 16x2 I2C LCD, and sends an SMS containing a Google Maps link to a pre-programmed emergency contact number using a SIM900 GSM module on a second UART channel.

The firmware handles multi-UART communication (HardwareSerial for GPS on UART1, SIM900 on UART2), I2C LCD output via the Wire library, and TinyGPS++ NMEA parsing for accurate latitude/longitude extraction. The project integrates four distinct hardware modules -- accelerometer, GPS receiver, GSM modem, and LCD display -- into a single ESP32-based safety platform, demonstrating sensor fusion, serial communication multiplexing, and real-time emergency response automation.

## Key Metrics
| Metric | Value |
|--------|-------|
| Crash Threshold | 2.5 G-force |
| Alert Method | GPS + SMS with Google Maps link |
| Display | I2C LCD 16x2 |
| Hardware Modules | 4 (Accelerometer, GPS, GSM, LCD) |

## Hardware Connection Table

| Component | ESP32 Pin | Interface | Protocol | Notes |
|-----------|-----------|-----------|----------|-------|
| ADXL335 X-Axis | GPIO34 | Analog (ADC) | 0-3.3V | 12-bit ADC, input-only |
| ADXL335 Y-Axis | GPIO35 | Analog (ADC) | 0-3.3V | 12-bit ADC, input-only |
| ADXL335 Z-Axis | GPIO32 | Analog (ADC) | 0-3.3V | 12-bit ADC, input-only |
| NEO-6M TX | GPIO16 | UART1 RX | 9600 baud | NMEA sentences |
| NEO-6M RX | GPIO17 | UART1 TX | 9600 baud | GPS module |
| SIM900 TX | GPIO5 | UART2 RX | 9600 baud | GSM modem |
| SIM900 RX | GPIO4 | UART2 TX | 9600 baud | SMS alerts |
| LCD SDA | GPIO21 | I2C | 400 kHz | PCF8574 backpack |
| LCD SCL | GPIO22 | I2C | 400 kHz | 16x2 display |
| ADXL335 VCC | 3.3V | Power | -- | Do NOT use 5V |
| NEO-6M VCC | 3.3V | Power | -- | 30-70 mA |
| SIM900 VCC | 5V | Power | -- | 2A peak (use external!) |

### Power Supply Requirements
| Component | Voltage | Current (peak) |
|-----------|---------|----------------|
| ESP32 | 3.3V | ~240 mA |
| ADXL335 | 3.3V | ~350 uA |
| NEO-6M | 3.3V | ~67 mA |
| SIM900 | 5V | 2A (during TX burst) |

> Use a 5V 3A external power supply. SIM900 draws 2A during transmission bursts!

## Multi-UART Architecture
```
UART0 (USB)  -> Serial Monitor (debug)
UART1 (GPIO16/17) -> NEO-6M GPS (TinyGPS++ NMEA parsing)
UART2 (GPIO4/5)   -> SIM900 GSM (AT commands, SMS)
I2C   (GPIO21/22)  -> LCD 16x2 (status display)
ADC   (GPIO32/34/35) -> ADXL335 (3-axis acceleration)
```

## Tech Stack
- **MCU**: ESP32 (Dual-core, 240 MHz)
- **Accelerometer**: ADXL335 (analog 3-axis, +/-3g)
- **GPS**: NEO-6M (UART, NMEA 0183)
- **GSM**: SIM900 (UART, AT commands)
- **Display**: 16x2 LCD + PCF8574 I2C backpack
- **Language**: C++ (Arduino framework)

## Author
**Mothi Charan Naik Desavath** -- Embedded Systems Engineer