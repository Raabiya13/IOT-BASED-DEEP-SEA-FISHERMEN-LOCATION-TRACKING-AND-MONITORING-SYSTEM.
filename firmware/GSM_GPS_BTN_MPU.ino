#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <MPU6050.h>

// Define serial pins for GSM
SoftwareSerial gsmSerial(10, 11); // RX, TX for GSM (use appropriate pins)

// Define serial pins for GPS
SoftwareSerial gpsSerial(4, 3); // RX, TX for GPS

TinyGPSPlus gps;
MPU6050 mpu;

double latitude = 0.0;
double longitude = 0.0;

bool alertSent = false; // Prevent multiple alerts for same tilt

void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  gsmSerial.begin(9600);

  Wire.begin();
  mpu.initialize();

  if (mpu.testConnection()) {
    Serial.println("MPU6050 connected successfully.");
  } else {
    Serial.println("MPU6050 connection failed.");
  }

  // Initialize GSM
  delay(1000);
  gsmSerial.println("AT");
  waitForResponse();
  gsmSerial.println("AT+CMGF=1"); // Set text mode
  waitForResponse();
  Serial.println("System Initialized...");
}

void loop() {
  // Read GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
    }
  }

  // Read MPU6050 data
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  Serial.print("Accel X: "); Serial.print(ax);
  Serial.print(" Y: "); Serial.print(ay);
  Serial.print(" Z: "); Serial.println(az);

  Serial.print("Gyro X: "); Serial.print(gx);
  Serial.print(" Y: "); Serial.print(gy);
  Serial.print(" Z: "); Serial.println(gz);

  // Tilt detection
  if ((abs(ax) > 15000 || abs(gy) > 20000 || abs(az) < 12000) && !alertSent) {
    Serial.println("🚨 Boat tilted! Sending alert...");
    sendSMS("+916385116645", "EMERGENCY ALERT: Fisherman at Latitude: " + String(latitude, 6) +
             " Longitude: " + String(longitude, 6) + ". Please assist.");
    alertSent = true;  // Prevent repeated alerts
  }

  // Reset alert if boat is back to normal
  if (abs(ax) < 12000 && abs(gy) < 10000 && abs(az) > 14000) {
    alertSent = false;
  }

  delay(1000);
}

void sendSMS(String number, String message) {
  gsmSerial.println("AT+CMGF=1");      // Set text mode
  delay(500);
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(number);
  gsmSerial.println("\"");
  delay(1000);

  gsmSerial.print(message);
  delay(500);
  gsmSerial.write(26); // Ctrl+Z to send
  delay(5000);
}

void waitForResponse() {
  long timeout = millis() + 5000;
  while (millis() < timeout) {
    if (gsmSerial.available()) {
      Serial.write(gsmSerial.read());
    }
  }
}
