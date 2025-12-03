#include <esp_now.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <NewPing.h>

// Board and pins
#define BOARD_ID 1
#define VIBRATION 32
#define BUZZER 12
#define IR 13
#define ALCOHOL 5
#define TRIG_PIN 14
#define ECHO_PIN 27
#define MAX_DISTANCE 200  // cm for ultrasonic

// NewPing instance
NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// Data structure to send
typedef struct struct_message {
    int id;
    int vibration;
    int ir;        // Helmet detection
    int alcohol;   // Alcohol sensor reading
    int distance;  // Ultrasonic distance
} struct_message;

struct_message myData;

// Receiver MAC Address (change to your receiver's MAC)
uint8_t broadcastAddress[] = {0xCC, 0xDB, 0xA7, 0x31, 0x97, 0x30};

// Interval between sends (ms)
const long interval = 100;
unsigned long previousMillis = 0;

// Send callback
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  pinMode(VIBRATION, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(IR, INPUT);
  pinMode(ALCOHOL, INPUT);

  // Set Wi-Fi in station mode
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE); // set same channel as receiver

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register send callback
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 6;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  Serial.println("ESP-NOW Sender ready!");
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    int vibration = digitalRead(VIBRATION);
    int ir = digitalRead(IR);             // Helmet detection
    int alcohol = digitalRead(ALCOHOL);    // Alcohol sensor (analog value)
    int distance = sonar.ping_cm();       // Ultrasonic distance in cm

    // Buzzer alerts
    if (alcohol == 0) {  // No helmet
      digitalWrite(BUZZER, HIGH);
    } else if (distance > 0 && distance < 20) { // Obstacle detected
      digitalWrite(BUZZER, HIGH);
    } else {
      digitalWrite(BUZZER, LOW);
    }

    // Prepare data
    myData.id = BOARD_ID;
    myData.vibration = vibration;
    myData.ir = ir;
    myData.alcohol = alcohol;
    myData.distance = distance;

    // Send with retry mechanism
    esp_err_t result;
    int attempts = 0;
    do {
      result = esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));
      attempts++;
      if(result == ESP_OK) break;
      delay(50); // wait before retry
    } while(attempts < 5);

    if(result == ESP_OK){
      Serial.println("Sent with success");
    } else {
      Serial.println("Error sending the data after 5 attempts");
    }

    // Debugging
    Serial.print("Vibration: "); Serial.print(vibration);
    Serial.print(" | IR (Helmet): "); Serial.print(ir);
    Serial.print(" | Alcohol: "); Serial.print(alcohol);
    Serial.print(" | Distance: "); Serial.println(distance);
  }
}
