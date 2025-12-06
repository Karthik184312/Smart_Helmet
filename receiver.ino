#define BLYNK_TEMPLATE_ID           "TMPL3VSWUwPvy"
#define BLYNK_TEMPLATE_NAME         "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "HdWDbbbc8E0hwhxLFeX78YH-cgrib2R7"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

#define RELAY 26
/* Comment this out to disable prints and save space */

#include <BlynkSimpleEsp32.h>
#include <esp_now.h>

#include <WiFi.h>



//#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>

#include <TinyGPS++.h>

HardwareSerial Gps(2);
String loc="";
TinyGPSPlus gps;

// Replace with your network credentials (STATION)
const char* ssid = "Lalitha";
const char* password = "sumukha193";



// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
    int vibration;
    int ir;        // Helmet detection
    int alcohol;   // Alcohol sensor reading
    int distance;  // Ultrasonic distance
  
  
} struct_message;

struct_message incomingReadings;


JSONVar board;
//
//AsyncWebServer server(80);
//AsyncEventSource events("/events");

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) { 
  // Copies the sender mac address to a string
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  
//  board["id"] = incomingReadings.id;
//  board["tilt"] = incomingReadings.tilt;
//  board["vibration"] = incomingReadings.vibration;
//  board["loc"] = incomingReadings.loc;
//  board["readingId"] = String(incomingReadings.readingId);
  String jsonString = JSON.stringify(board);
//  events.send(jsonString.c_str(), "new_readings", millis());
  
  Serial.print("Board ID :"+String(incomingReadings.id));
  //Serial.print("t tilt: " +String(incomingReadings.tilt));
  Serial.print("h vibration:" + String(incomingReadings.vibration));
//   Serial.print("h location: "+String( incomingReadings.loc));
  


 


  int vibration=incomingReadings.vibration;

  int ir=incomingReadings.ir;

  int al=incomingReadings.alcohol;
  
  Serial.println("Vibration = "+String(vibration));
  Serial.println("IR = "+String(ir));


  if( ir==0 )
  {
    digitalWrite(RELAY,LOW);
  }else{
    
    digitalWrite(RELAY,HIGH);
    }


  if( al==0 )
  {
    digitalWrite(RELAY,HIGH);

    Serial.println("Alchohol consumption detected !");
    //Blynk.logEvent("alert","Alchohol detected");
  }


  if( vibration==0 )
  {


    Serial.println("Accident Detected");

    Blynk.logEvent("alert","Accident detected at location : "+loc);
  }
  
  


  
}



void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  pinMode(RELAY,OUTPUT);
  digitalWrite(RELAY,HIGH);
  
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  Gps.begin(9600);
  // Set the device as a Station and Soft Access Point simultaneously
  WiFi.mode(WIFI_AP_STA);
  
  // Set device as a Wi-Fi Station
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info

  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  esp_now_register_recv_cb(OnDataRecv);


 
//  timer.setInterval(1000l,myTimerEvent);   
}
 
void loop() {

//  static unsigned long lastEventTime = millis();
//  static const unsigned long EVENT_INTERVAL_MS = 5000;
//  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
//    events.send("ping",NULL,millis());
//    lastEventTime = millis();
//  


 if (Gps.available())
  {
      Serial.println("GPS avilable");
      
      if ( gps.encode(Gps.read())){
      
      loc=String(gps.location.lat(),6)+","+String(gps.location.lng(),6);
      Serial.println(loc);
        

  }}
Blynk.run();

}
