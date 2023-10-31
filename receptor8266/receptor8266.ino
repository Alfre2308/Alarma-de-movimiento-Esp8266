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

int outSound = 5;
int ledWork = 4;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  bool detect;
} struct_message;

// Create a struct_message called myData
struct_message myData;

// Variable para llevar un registro del estado del LED
bool ledState = LOW;
unsigned long lastTime = 0;  
unsigned long timerDelay = 2300; 
unsigned long timerDelay2 = 2000; 

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t* mac, uint8_t* incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.print("Bytes received: ");
  Serial.println(myData.detect);

  // Cambiar el estado del LED
  ledState = !ledState;
  digitalWrite(ledWork, ledState);
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

  // Configurar el pin del LED ledWork como salida
  pinMode(ledWork, OUTPUT);
    pinMode(outSound, OUTPUT);
     digitalWrite(outSound, LOW);
  digitalWrite(ledWork, LOW); // Apagar el LED inicialmente
}
void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if(myData.detect){
    digitalWrite(outSound, HIGH);
    delay(2000);
    digitalWrite(outSound, LOW);
  }
      lastTime = millis();
  }
 
     

}