#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

const char* ssid = "SDA_System";
const char* password = "Alexs.1008";
bool soundEnabled = true;

AsyncWebServer server(80);
const uint16_t webSocketPort = 81;
WebSocketsServer webSocket = WebSocketsServer(webSocketPort);


int outSound = 4;
int ledWork0 = 26;
int ledWork1 = 25;
const int maxClients = 10;
const char* styles = "<style>"
                      "body { background-color: lightgray; font-family: Arial, sans-serif; }"
                      "h1 { color: red; text-align: center; font-size: 36px; }"
                      "p { color: blue; font-size: 24px; }"
                      "</style>";



// Estructura para almacenar los datos de cada sensor
struct SensorData {
  uint8_t mac[6];
  bool detect;
  char sensorName[32];
  long int cantMensCorrect;
  long int cantMensInCorrect;
};
unsigned long soundStateDuration = 200;
unsigned long lastStateChangeTime = 0;
bool soundState=LOW;
const char* html = R"html(
<!DOCTYPE HTML>
<html>
  <head>
    <style>
      * {
        box-sizing: border-box;
      }
      
      body {
        background-color: #f5f5f5;
        font-family: Arial, sans-serif;
        margin: 0;
        padding: 20px;
      }
      
      h1 {
        color: #221a8f;
        text-align: center;
        font-size: 42px;
        margin-bottom: 30px;
      }
      
      p {
        color: #0f4d25;
        font-size: 26px;
        margin: 10px 0;
      }
      
      .sensor-container {
        border: 1px solid #ddd;
        padding: 20px;
        border-radius: 5px;
        margin-bottom: 20px;
      }
      
      .sensor-heading {
        color: #0f4d25;
        font-weight: bold;
        font-size: 42px;
        margin-bottom: 10px;
      }
      
      .sensor-status {
        font-size: 24px;
      }
      
      .sensor-detectado {
        color: green;
      }
      
      .sensor-no-detectado {
        color: red;
      }
      
      /* Estilos CSS para el botón */
      .sound-button {
        font-size: 24px; /* Tamaño del texto */
        padding: 10px 20px; /* Espacio interno del botón */
        background-color: #4CAF50; /* Color de fondo normal */
        color: white; /* Color del texto */
        border: none;
        cursor: pointer;
        position: absolute; /* Posición absoluta */
        top: 20px; /* Distancia desde la parte superior */
        right: 20px; /* Distancia desde la parte derecha */
      }

      /* Estilo para el botón desactivado (soundEnabled = false) */
      .sound-button.disabled {
        background-color: #ccc; /* Cambiar el color de fondo cuando está desactivado */
        cursor: not-allowed; /* Cambiar el cursor cuando está desactivado */
      }
    </style>
    <script>
    var webSocket;
    var soundEnabled=true;
    function setupWebSocket() {
        webSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
        
        webSocket.onopen = function(event) {
          console.log('WebSocket connection established');
        };
        
        webSocket.onmessage = function(event) {
          var sensorData = JSON.parse(event.data);
          updateSensorStates(sensorData);
        };
        
        webSocket.onclose = function(event) {
          console.log('WebSocket connection closed');
          setTimeout(setupWebSocket, 2000);
        };
      }
      
      function toggleSound() {
  soundEnabled = !soundEnabled; // Invertir el estado del sonido
  var button = document.getElementById("toggleSoundButton");
  button.innerHTML = soundEnabled ? "Sonido On" : "Sonido Off";

  // Aplicar estilos según el estado del sonido
  button.classList.remove(soundEnabled ? "disabled" : "enabled");
  button.classList.add(soundEnabled ? "enabled" : "disabled");

  // Envía la información al servidor a través de WebSocket para sincronizar el estado
  var data = { soundEnabled: soundEnabled };
  webSocket.send(JSON.stringify(data));
}
      
    function updateSensorStates(sensorData) {
  for (var i = 0; i < sensorData.length; i++) {
    var sensorState = sensorData[i].detect ? 'Detectado' : 'No Detectado';
    var name = sensorData[i].name;
    var correcto = sensorData[i].correcto;
    var incorrecto = sensorData[i].incorrecto;
    
    var sensorElement = document.getElementById('sensor_' + i);
    var datosCorrectos = document.getElementById('sensor_' + i +'_correctos');
    var datosInCorrectos = document.getElementById('sensor_' + i +'_incorrectos');
    // var sensorMacElement = document.getElementById('sensor_' + i + '_mac');
    sensorElement.innerHTML = name + ': ' + sensorState;
    datosCorrectos.innerHTML = 'Envios Correctos: '+ correcto;
    datosInCorrectos.innerHTML = 'Envios Incorrectos: '+ incorrecto;  
    sensorElement.classList.remove('sensor-detectado', 'sensor-no-detectado');
    sensorElement.classList.add(sensorData[i].detect ? 'sensor-detectado' : 'sensor-no-detectado');
  }
}
      
      window.onload = function() {
        setupWebSocket();
      };
    </script>
  </head>
  <body>
    <h1>Sistema de Alarma</h1>
    
<div class="sensor-container">
  <div class="sensor-heading">Sensor 0</div>
  <p id="sensor_0" class="sensor-status">Estado:</p>
   <p id="sensor_0_correctos" class="sensor-correctos">Envios Correctos:</p>
  <p id="sensor_0_incorrectos" class="sensor-incorrectos">Envios Incorrectos:</p>
 
</div>
    
    <div class="sensor-container">
      <div class="sensor-heading">Sensor 1</div>
      <p id="sensor_1" class="sensor-status">Estado:</p>
       <p id="sensor_1_correctos" class="sensor-correctos">Envios Correctos:</p>
  <p id="sensor_1_incorrectos" class="sensor-incorrectos">Envios Incorrectos:</p>
    </div>
    
    <div class="sensor-container">
      <div class="sensor-heading">Sensor 2</div>
      <p id="sensor_2" class="sensor-status">Estado:</p>
       <p id="sensor_2_correctos" class="sensor-correctos">Envios Correctos:</p>
  <p id="sensor_2_incorrectos" class="sensor-incorrectos">Envios Incorrectos:</p>
    </div>
   <button id="toggleSoundButton" onclick="toggleSound()" class="sound-button">
  Sonido On
</button>

<style>
  /* Estilos CSS para el botón */
  .sound-button {
    font-size: 24px; /* Tamaño del texto */
    padding: 10px 20px; /* Espacio interno del botón */
    background-color: #4CAF50; /* Color de fondo normal */
    color: white; /* Color del texto */
    border: none;
    cursor: pointer;
  }

  /* Estilo para el botón presionado (soundEnabled = false) */
  .sound-button.disabled {
    background-color: #ccc; /* Cambiar el color de fondo cuando está desactivado */
    cursor: not-allowed; /* Cambiar el cursor cuando está desactivado */
  }
</style>
    <!-- Agrega más elementos <div> para otros sensores si es necesario -->
  </body>
</html>
)html";
void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch(type) {
    // Evento de conexión
    case WStype_CONNECTED:
      Serial.printf("[%u] Conectado al cliente\n", num);
      break;
    // Evento de desconexión
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Desconectado del cliente\n", num);
      break;
    // Evento de mensaje recibido
    case WStype_TEXT:
StaticJsonDocument<256> doc;

      // Deserializar el payload JSON
      DeserializationError error = deserializeJson(doc, payload, length);

      // Verificar si hubo un error en la deserialización
      if (error) {
        Serial.print("Error al deserializar el JSON: ");
        Serial.println(error.c_str());
        return;
      }

      // Obtener el valor de la propiedad "soundEnabled"
       soundEnabled = doc["soundEnabled"];

      // Imprimir el valor de "soundEnabled"
      Serial.print("soundEnabled: ");
      Serial.println(soundEnabled);



      break;
  }
}

SensorData sensorData[maxClients]; // Matriz para almacenar los datos de los sensores
int numSensors = 0; // Número actual de sensores

bool ledState0 = LOW;
bool ledState1 = LOW;
unsigned long lastTime = 0;
unsigned long timerDelay = 3000;
unsigned long webUpdateInterval = 1000; // Intervalo para actualizar la página web (1 segundo)

void onDataReceiver(const uint8_t* mac, const uint8_t* incomingData, int len) {
  if (numSensors < maxClients) {
    // Buscar el sensor en la lista
    int sensorIndex = -1;
    for (int i = 0; i < numSensors; i++) {
      if (memcmp(mac, sensorData[i].mac, 6) == 0) {
        sensorIndex = i;
        break;
      }
    }

    // Si no se encuentra en la lista, agregarlo como un nuevo sensor
    if (sensorIndex == -1) {
      memcpy(sensorData[numSensors].mac, mac, 6);
      numSensors++;
    }

    // Actualizar el estado de detección del sensor
    if (sensorIndex != -1) {
           memcpy(&sensorData[sensorIndex], incomingData, sizeof(sensorData[sensorIndex]));
      // memcpy(&sensorData[sensorIndex].detect, incomingData, sizeof(sensorData[sensorIndex].detect));
      //  memcpy(&sensorData[sensorIndex].cantMensCorrect, incomingData, sizeof(sensorData[sensorIndex].cantMensCorrect));
      //   memcpy(&sensorData[sensorIndex].cantMensInCorrect, incomingData, sizeof(sensorData[sensorIndex].cantMensInCorrect));

      //  Serial.print(sensorData[sensorIndex].cantMensCorrect);
      // Serial.print(sensorData[sensorIndex].cantMensInCorrect);
      Serial.print(sensorData[sensorIndex].sensorName);
      Serial.print(" - Detección: ");
      Serial.println(sensorData[sensorIndex].detect);
 if (sensorIndex == 0) {
  ledState0 = !ledState0;
  digitalWrite(ledWork0, ledState0);}
  if (sensorIndex == 1) {
  ledState1 = !ledState1;
  digitalWrite(ledWork1, ledState1);}
    
    }
    
    }
 
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.println(IP);

   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", html);
  });


  server.begin();
  webSocket.begin();
  webSocket.onEvent(handleWebSocketEvent);

  if (esp_now_init() != 0) {
    Serial.println("Problem during ESP-NOW init");
    return;
  }
  esp_now_register_recv_cb(onDataReceiver);

  pinMode(ledWork0, OUTPUT);
  pinMode(ledWork1, OUTPUT);
  pinMode(outSound, OUTPUT);
  digitalWrite(outSound, LOW);
  digitalWrite(ledWork0, LOW);
    digitalWrite(ledWork1, LOW);
}

void loop() {
  // Serial.println("Sonido:"+soundEnabled);
   webSocket.loop();
   DynamicJsonDocument jsonDocument(512);
    JsonArray sensorArray = jsonDocument.to<JsonArray>();
    
    // Agregar los datos de cada sensor al objeto JSON
    for (int i = 0; i < numSensors; i++) {
      JsonObject sensorObject = sensorArray.createNestedObject();
      sensorObject["detect"] = sensorData[i].detect;
      sensorObject["name"] = sensorData[i].sensorName;
      sensorObject["correcto"] = sensorData[i].cantMensCorrect;
      sensorObject["incorrecto"] = sensorData[i].cantMensInCorrect;
    }
    
    // Convertir el objeto JSON en una cadena y enviarla a través del WebSocket
    String sensorDataString;
    serializeJson(sensorArray, sensorDataString);
    webSocket.broadcastTXT(sensorDataString);

  static unsigned long lastWebUpdateTime = 0;
  if (millis() - lastWebUpdateTime >= webUpdateInterval) {
    // handleWebPage(); // Actualiza la página web a intervalos regulares
    lastWebUpdateTime = millis();
  }


//  if (soundEnabled) {
//   for (int i = 0; i < numSensors; i++) {
//     if (sensorData[i].detect) {
//       digitalWrite(outSound, HIGH);
//       lastTime = millis(); // Registra el tiempo de la última detección
//     }
//   }

//   if (millis() - lastTime >= timerDelay) {
//     digitalWrite(outSound, LOW); // Apaga la salida después de 3 segundos
//   }
//  }
//  else{
//   digitalWrite(outSound, LOW); 
//  }

if (soundEnabled) {
    for (int i = 0; i < numSensors; i++) {
      if (sensorData[i].detect) {
        lastTime = millis(); // Registra el tiempo de la última detección
        soundState = HIGH; // Establece el estado en alto
        lastStateChangeTime = millis();
        digitalWrite(outSound, soundState);
      }
    }

    // Verifica si ha pasado suficiente tiempo para cambiar el estado
    if (millis() - lastStateChangeTime >= soundStateDuration) {
      soundState = !soundState; // Invierte el estado (alto a bajo o viceversa)
      lastStateChangeTime = millis();
      digitalWrite(outSound, soundState);
    }

    // Verifica si ha pasado suficiente tiempo desde la última detección
    if (millis() - lastTime >= timerDelay) {
      digitalWrite(outSound, LOW); // Apaga la salida después de 3 segundos
    }
  } else {
    digitalWrite(outSound, LOW);
  }
}

