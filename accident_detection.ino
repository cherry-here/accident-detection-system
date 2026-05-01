/*
 * Accident Detection & Alert System
 * Author: Mothi Charan Naik Desavath
 * Hardware: Arduino Uno + MPU6050 + SIM800L GSM + NEO-6M GPS
 * Detects crash via accelerometer, sends SMS with GPS location
 */

#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

MPU6050 mpu;
SoftwareSerial gsmSerial(7, 8);   // GSM TX, RX
SoftwareSerial gpsSerial(4, 5);   // GPS TX, RX
TinyGPSPlus gps;

#define BUZZER_PIN 9
#define LED_PIN 13
#define ALERT_NUMBER "+91XXXXXXXXXX"  // Emergency contact number
#define IMPACT_THRESHOLD 2.5          // G-force threshold for accident detection
#define CONFIRM_DELAY 5000            // 5 sec wait before sending alert

bool accidentDetected = false;
unsigned long accidentTime = 0;

void setup() {
    Serial.begin(9600);
    gsmSerial.begin(9600);
    gpsSerial.begin(9600);
    Wire.begin();
    mpu.initialize();
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);
    Serial.println("Accident Detection System Ready");
}

void loop() {
    // Read GPS
    while (gpsSerial.available()) gps.encode(gpsSerial.read());

    // Read accelerometer
    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    float gForce = sqrt(pow(ax/16384.0, 2) + pow(ay/16384.0, 2) + pow(az/16384.0, 2));

    if (gForce > IMPACT_THRESHOLD && !accidentDetected) {
        accidentDetected = true;
        accidentTime = millis();
        triggerAlert();
    }

    // Confirm after delay (avoids false positives)
    if (accidentDetected && millis() - accidentTime > CONFIRM_DELAY) {
        sendSMSAlert();
        accidentDetected = false;
    }
}

void triggerAlert() {
    digitalWrite(LED_PIN, HIGH);
    tone(BUZZER_PIN, 1000, 3000);
    Serial.println("IMPACT DETECTED! Sending alert in 5 seconds...");
}

void sendSMSAlert() {
    String lat = gps.location.isValid() ? String(gps.location.lat(), 6) : "Unknown";
    String lng = gps.location.isValid() ? String(gps.location.lng(), 6) : "Unknown";
    String msg = "ACCIDENT ALERT! Location: https://maps.google.com/?q=" + lat + "," + lng;

    gsmSerial.println("AT+CMGF=1");
    delay(500);
    gsmSerial.println("AT+CMGS=\"" + String(ALERT_NUMBER) + "\"");
    delay(500);
    gsmSerial.println(msg);
    delay(500);
    gsmSerial.write(26);  // Ctrl+Z to send

    Serial.println("SMS Sent: " + msg);
    digitalWrite(LED_PIN, LOW);
}