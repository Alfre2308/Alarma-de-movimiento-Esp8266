/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp8266-nodemcu-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xC4, 0x4F, 0x33, 0x75, 0x85, 0x35};

// Structure example to send data
// Must match the receiver structure
 int inputPir=5;
 const int analogInPin = A0;
 int sensorValue = 0;
 int lecturaAcumulada=0;
typedef struct struct_message { 
    uint8_t mac[6]={0xDC, 0x4F, 0x22, 0x7E, 0xE3, 0xDC};
  bool detect;
  char sensorName[32];
  long int cantMensCorrect;
  long int cantMensInCorrect;
} struct_message;

// Create a struct_message called myData
struct_message myData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 1300;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
     myData.cantMensCorrect=myData.cantMensCorrect+1;
  }
  else{
    Serial.println("Delivery fail");
    myData.cantMensInCorrect=myData.cantMensInCorrect+1;
  }
}
 
void setup() {
  strcpy( myData.sensorName ,"Sala");
  // Init Serial Monitor
  Serial.begin(115200);
 pinMode(inputPir, INPUT);  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);
}
 
void loop() {
  delay(20);
  
 
  sensorValue = analogRead(analogInPin);
// Serial.println(sensorValue);
// Serial.println(digitalRead(inputPir));
  if(sensorValue>lecturaAcumulada){
    lecturaAcumulada=sensorValue;
  }
  //  Serial.println(lecturaAcumulada);
  if(lecturaAcumulada>82 || digitalRead(inputPir)==1){
   myData.detect = 1;
  }
  else{
     myData.detect = 0; 
  }
  if ((millis() - lastTime) > timerDelay) {
    // Set values to send        

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    lastTime = millis();
    lecturaAcumulada=0;
  }
}