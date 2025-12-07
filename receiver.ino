#define BLYNK_TEMPLATE_ID "TMPL3VSWUwPvy"
#define BLYNK_TEMPLATE_NAME "Quickstart Device"
#define BLYNK_AUTH_TOKEN "HdWDbbbc8E0hwhxLFeX78YH-cgrib2R7"

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <esp_now.h>
#include <BlynkSimpleEsp32.h>
#include <TinyGPS++.h>

#define RELAY 26

TinyGPSPlus gps;
HardwareSerial Gps(2);
String loc = "";

// WiFi details
const char* ssid = "vivo";
const char* password = "01234567";

// --------------- Data Structure ---------------
typedef struct struct_message {
  int id;
  int vibration;
  int ir;
  int alcohol;
  int distance;
} struct_message;

struct_message incomingReadings;

// --------------- ESP-NOW Callback ---------------
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingData, int len) {
  
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));

  Serial.print("Data Received from: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", info->src_addr[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();

  Serial.println("ID = " + String(incomingReadings.id));
  Serial.println("Vibration = " + String(incomingReadings.vibration));
  Serial.println("IR = " + String(incomingReadings.ir));
  Serial.println("Alcohol = " + String(incomingReadings.alcohol));

  int vibration = incomingReadings.vibration;
  int ir = incomingReadings.ir;
  int al = incomingReadings.alcohol;

  // Helmet detection
  if (ir == 0) digitalWrite(RELAY, LOW);
  else digitalWrite(RELAY, HIGH);

  // Alcohol detection
  if (al == 0) {
    Serial.println("Alcohol detected!");
    digitalWrite(RELAY, LOW);
    Blynk.logEvent("alert", "Alcohol detected!");
  }

  // Accident detection
  if (vibration == 0) {
    Serial.println("Accident Detected!");
    Blynk.logEvent("alert", "Accident detected at location: " + loc);
  }
}

void setup() {
  Serial.begin(115200);
  Gps.begin(9600);

  pinMode(RELAY, OUTPUT);
  digitalWrite(RELAY, HIGH);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);

  WiFi.mode(WIFI_STA);
  delay(100);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  Serial.println("ESP-NOW Ready (ESP32-S3)");
}

void loop() {

  // GPS Reading
  if (Gps.available()) {
    if (gps.encode(Gps.read())) {
      if (gps.location.isValid()) {
        loc = String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6);
        Serial.println("GPS: " + loc);
      }
    }
  }

  Blynk.run();
}

