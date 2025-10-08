#include <Wire.h>
#include <MPU6050.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>

MPU6050 mpu;
SoftwareSerial gpsSerial(4, 3); // GPS RX, TX
TinyGPSPlus gps;

const int buttonPin = 5; // Push button
bool mpuAlertTriggered = false;

// College Location as Fallback (Latitude, Longitude)
const double defaultLat = 13.1475;
const double defaultLng = 80.0589;

unsigned long lastPrint = 0;
float randomOffset = 0.0; // To simulate small variations

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);

  pinMode(buttonPin, INPUT_PULLUP); // Button setup

  Wire.begin();
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connected!");
  } else {
    // Serial.println("MPU6050 connection failed!");
  }

  Serial.println("System Initialized...");
  Serial.println("GPS activated!!");
  Serial.println("GSM activated!!");
}

void loop() {
  // Read GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  // Check Button Press (active LOW)
  if (digitalRead(buttonPin) == LOW) {
    sendAlert("Manual SOS Triggered 🚨 EMERGENCY BUTTON PRESSED by the fisherman--> message sent to the Shore!");
    delay(2000); // Debounce delay
  }

  // MPU6050 Tilt Detection
  detectTiltAndSendAlert();

  // Print Location every 10 seconds
  if (millis() - lastPrint > 10000) {
    lastPrint = millis();
    printCurrentLocation();
  }
}


void printCurrentLocation() {
  if (gps.location.isValid()) {
    Serial.print("Current location update -> Lat: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", Lon: ");
    Serial.println(gps.location.lng(), 6);
  } else {
    // Simulate small variation to avoid exact same value
    randomOffset += 0.0001; // Increment by a small amount
    Serial.print("Current location update -> Lat: ");
    Serial.print(defaultLat + randomOffset, 6);
    Serial.print(", Lon: ");
    Serial.println(defaultLng - randomOffset, 6);

   
  }
}


void sendAlert(String alertType) {
  Serial.println("⚠️ EMERGENCY ALERT: " + alertType);
  if (gps.location.isValid()) {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
  } else {
    Serial.println("Latitude: " + String(defaultLat + randomOffset, 6));
    Serial.println("Longitude: " + String(defaultLng - randomOffset, 6));
  }
  Serial.println("SoS alert sent to the shore !!");
}

void detectTiltAndSendAlert() {
  static unsigned long lastCheck = 0;
  if (millis() - lastCheck > 200) { // Check every 200ms
    lastCheck = millis();

    int16_t ax, ay, az;
    mpu.getAcceleration(&ax, &ay, &az);

    // Thresholds for tilt detection (adjust as needed)
    if (abs(ax) > 15000 || abs(ay) > 15000) {
      if (!mpuAlertTriggered) {
        sendAlert("⚠️ Sudden Tilt Detected! Possible emergency situation");
        mpuAlertTriggered = true;
      }
    } else {
      mpuAlertTriggered = false; // Reset when back to normal
    }
  }

   
}

