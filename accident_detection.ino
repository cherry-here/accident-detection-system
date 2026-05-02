/*
 * Accident Detection & Alert System -- GPS + GSM SMS
 * Author: Mothi Charan Naik Desavath
 * Hardware: ESP32 + ADXL335 + NEO-6M GPS + SIM900 GSM + I2C LCD 16x2
 * Features: 3-axis G-force crash detection, GPS location, instant SMS alert
 *
 * Multi-UART Architecture:
 *   UART0 (USB Serial) -- Debug output
 *   UART1 (GPIO16/GPIO17) -- NEO-6M GPS (9600 baud)
 *   UART2 (GPIO4/GPIO5) -- SIM900 GSM (9600 baud)
 */

#include <WiFi.h>
#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- Pin Definitions -------------------------------------
// ADXL335 Analog Outputs
#define ADXL_X_PIN   34     // X-axis acceleration
#define ADXL_Y_PIN   35     // Y-axis acceleration
#define ADXL_Z_PIN   32     // Z-axis acceleration

// I2C LCD
#define LCD_ADDR     0x27   // PCF8574 I2C address
#define LCD_SDA      21     // I2C Data
#define LCD_SCL      22     // I2C Clock

// GSM Emergency Contact
#define ALERT_NUMBER "+91XXXXXXXXXX"

// --- Detection Parameters --------------------------------
#define CRASH_THRESHOLD   2.5    // G-force threshold
#define GPS_TIMEOUT       5000   // Max ms to wait for GPS fix
#define SMS_RETRY_COUNT   3      // SMS send retries

// --- Serial Ports ----------------------------------------
HardwareSerial gpsSerial(1);    // UART1: GPS (GPIO16 TX, GPIO17 RX)
HardwareSerial gsmSerial(2);    // UART2: GSM (GPIO4 TX, GPIO5 RX)

// --- Objects ---------------------------------------------
TinyGPSPlus gps;
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

// --- State Variables -------------------------------------
bool crashDetected = false;
unsigned long crashTime = 0;

void setup() {
    // Debug serial
    Serial.begin(115200);
    Serial.println("Accident Detection System Starting...");

    // GPS serial (9600 baud)
    gpsSerial.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17

    // GSM serial (9600 baud)
    gsmSerial.begin(9600, SERIAL_8N1, 5, 4);    // RX=5, TX=4

    // I2C LCD
    Wire.begin(LCD_SDA, LCD_SCL);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(0, 0);
    lcd.print("Accident Detect");
    lcd.setCursor(0, 1);
    lcd.print("System Ready");

    // Initialize GSM
    initGSM();

    Serial.println("System Ready -- Monitoring for crashes...");
}

void loop() {
    // --- Read GPS continuously ------------------------
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
    }

    // --- Read Accelerometer & Calculate G-Force -------
    int rawX = analogRead(ADXL_X_PIN);
    int rawY = analogRead(ADXL_Y_PIN);
    int rawZ = analogRead(ADXL_Z_PIN);

    // Convert 12-bit ADC (0-4095) to voltage (0-3.3V)
    float vx = (rawX / 4095.0) * 3.3;
    float vy = (rawY / 4095.0) * 3.3;
    float vz = (rawZ / 4095.0) * 3.3;

    // ADXL335 sensitivity: ~300 mV/g at 3.3V supply
    // Zero-g offset: ~1.65V
    float ax = (vx - 1.65) / 0.300;  // G-force on X
    float ay = (vy - 1.65) / 0.300;  // G-force on Y
    float az = (vz - 1.65) / 0.300;  // G-force on Z

    float gForce = sqrt(ax * ax + ay * ay + az * az);

    // --- Crash Detection ------------------------------
    if (gForce > CRASH_THRESHOLD && !crashDetected) {
        crashDetected = true;
        crashTime = millis();
        triggerCrashAlert(gForce);
    }

    // --- Reset after 30 seconds -----------------------
    if (crashDetected && millis() - crashTime > 30000) {
        crashDetected = false;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("System Reset");
        lcd.setCursor(0, 1);
        lcd.print("Monitoring...");
        Serial.println("System reset -- monitoring resumed");
    }
}

void triggerCrashAlert(float gForce) {
    Serial.printf("\n!!! CRASH DETECTED -- %.1f G !!!\n", gForce);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("CRASH DETECTED!");
    lcd.setCursor(0, 1);
    lcd.print("G: ");
    lcd.print(gForce, 1);
    lcd.print(" | Alerting");

    // Wait briefly for GPS fix after crash
    unsigned long gpsStart = millis();
    while (!gps.location.isValid() && millis() - gpsStart < GPS_TIMEOUT) {
        while (gpsSerial.available()) gps.encode(gpsSerial.read());
    }

    // Get coordinates
    String lat, lng;
    if (gps.location.isValid()) {
        lat = String(gps.location.lat(), 6);
        lng = String(gps.location.lng(), 6);
        Serial.printf("Location: %s, %s\n", lat.c_str(), lng.c_str());
    } else {
        lat = "Unknown";
        lng = "Unknown";
        Serial.println("GPS fix unavailable -- sending alert without location");
    }

    // Build Google Maps URL
    String mapsUrl = "https://maps.google.com/?q=" + lat + "," + lng;

    // Send SMS alert (with retries)
    bool sent = false;
    for (int retry = 0; retry < SMS_RETRY_COUNT && !sent; retry++) {
        sent = sendSMS(mapsUrl);
        if (!sent) delay(2000);
    }

    if (sent) {
        Serial.println("SMS alert sent successfully!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ALERT SENT!");
        lcd.setCursor(0, 1);
        lcd.print("Check SMS");
    } else {
        Serial.println("SMS FAILED after retries!");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("SMS FAILED!");
    }
}

bool sendSMS(String mapsUrl) {
    String msg = "ACCIDENT ALERT! Location: " + mapsUrl;

    gsmSerial.println("AT+CMGF=1");       // Text mode
    delay(500);
    gsmSerial.println("AT+CMGS=\"" + String(ALERT_NUMBER) + "\"");
    delay(500);
    gsmSerial.print(msg);                 // Message body
    delay(100);
    gsmSerial.write(26);                  // Ctrl+Z to send
    delay(3000);

    // Check response
    if (gsmSerial.available()) {
        String response = gsmSerial.readString();
        if (response.indexOf("OK") != -1) return true;
    }
    return false;
}

void initGSM() {
    Serial.print("Initializing GSM");
    gsmSerial.println("AT");
    delay(1000);

    int attempts = 0;
    while (attempts < 10) {
        gsmSerial.println("AT");
        delay(500);
        if (gsmSerial.available()) {
            String resp = gsmSerial.readString();
            if (resp.indexOf("OK") != -1) {
                Serial.println(" -- GSM Ready!");
                gsmSerial.println("AT+CMGF=1");  // Text mode
                delay(500);
                return;
            }
        }
        Serial.print(".");
        attempts++;
        delay(1000);
    }
    Serial.println(" -- GSM Init FAILED!");
}