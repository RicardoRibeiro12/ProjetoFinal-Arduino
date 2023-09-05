#include <PubSubClient.h>
#include <DHT.h>
#include <String.h>
#include <DNSServer.h>
#include <Arduino.h>
#include <Hash.h>
#include <Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
//----------------------------
#include <LittleFS.h>
#include "FS.h"
//----------------------------
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
WiFiClient client;  // Criação da variável client do tipo WiFiClient
// Create an instance of the PubSubClient class
PubSubClient mqttclient(client);
const char* mqtt_server = "192.168.1.151";
const int mqtt_port = 1883;
const char* mqtt_topic = "1"; // Replace with the topic you want to subscribe to
//----------------------------


//DHT
#define DHTPIN 0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
float t = 0.0;
float h = 0.0;
unsigned long previousMillisSensor = 0;
const long intervalSensor = 2000;
//LDR
#define LDRPIN A0
float l = 0.0;
//RELE
// Set RELE GPIO
const int relePin13 = 13;
const int relePin15 = 15;
// Stores RELE state
String releState13;
String releState15;
//----------------------------
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
// HTTP POST request WFIMANAGER
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
// HTTP POST request Index
const char* PARAM_INPUT_3 = "serverIp";
const char* PARAM_INPUT_4 = "token";
//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String serverIp;
//campos da plataforma
String token;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* serverIpPath = "/server.txt";
const char* tokenIpPath = "/token.txt";

const char *host = "http://192.168.1.151/api/obsdatas";   //your IP/web server address
// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  //interval to wait for Wi-Fi connection (milliseconds)

// MQTT values
const char* mqttServer = "192.168.1.151";
const int mqttPort = 1883;
//id do controlador na plataforma
const char* mqttTopic = "1";



unsigned long previousMillisMqtt = 0;
unsigned long previousMillisHttp = 0;
const unsigned long mqttInterval = 1000; // Adjust as needed
const unsigned long httpInterval = 150000; // Adjust as needed

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}
// Read File from LittleFS
String readFile(fs::FS& fs, const char* path) {
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- failed to open file for reading");
    return String();
  }
  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    Serial.println("----------EU ESTOU AQUI A LER OS FICHEIROS----------");
    break;
  }
  return fileContent;
}
// Write file to LittleFS
void writeFile(fs::FS& fs, const char* path, const char* message) {
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

bool initWiFi() {
  if (ssid == "") {
    Serial.println("Undefined SSID");
    return false;
  }
  WiFi.mode(WIFI_STA);
  //localIP.fromString(ip.c_str());
  WiFi.localIP().toString();
  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
    Serial.println("Tentar dar Connect");
  }

  ip = WiFi.localIP().toString();
  Serial.println(ip);
  writeFile(LittleFS, ipPath, ip.c_str());
  //-----------
  WiFi.softAP("ESP-WIFI-MANAGER", NULL);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);



  if (ip != "")
  {
    Serial.println("ESTE E O IP NA REDE " + ip);
    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->redirect("http://" + ip + "/home");
    });


  }
  delay(5000);
  return true;
}





// Initialize WiFi

//----------------------------
String processor(const String& var) {
  //Serial.println(var);
  if (var == "STATE13") {
    if (digitalRead(relePin13)) {
      releState13 = "ON";
    }
    else {
      releState13 = "OFF";
    }
    return releState13;
  }
  else if (var == "STATE15") {
    if (digitalRead(relePin15)) {
      releState15 = "ON";
    }
    else {
      releState15 = "OFF";
    }
    return releState15;
  }
  else if (var == "TEMPERATURE") {
    return String(t);
  }
  else if (var == "HUMIDITY") {
    return String(h);
  }
  else if (var == "LDR") {
    return String(l);
  }
  else if (var == "IP") {
    return String(ip);
  }
  else if (var == "SSID") {
    return String(ssid);
  }
  else if (var == "PASS") {
    return String(pass);
  }
  else if (var == "SERVER") {
    return String(serverIp);
  }
  else if (var == "TOKEN") {
    return String(token);
  }
  return String();
}
void setup() {

  Serial.begin(115200);
  //----------------------------
  initLittleFS();
  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  serverIp = readFile(LittleFS, serverIpPath);
  token = readFile(LittleFS, tokenIpPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(serverIp);
  Serial.println(token);
  //----------------------------
  dht.begin();
  pinMode(relePin13, OUTPUT);
  digitalWrite(relePin13, LOW);
  pinMode(relePin15, OUTPUT);
  digitalWrite(relePin15, LOW);

  mqttclient.setServer(mqttServer, mqttPort);
  mqttclient.setCallback(callback);
  mqttclient.subscribe("1");

  HTTPClient http;

  if (initWiFi()) {
    Serial.println("Já vim com o initwifi a true");
    //--------------

    /*  while (!mqttclient.connected()) {
      Serial.println("Connecting to MQTT broker...");
      if (mqttclient.connect("ESP8266Client")) {
        Serial.println("Connected to MQTT broker");
        Serial.println("subscibing");
        Serial.println(mqttclient.subscribe(mqttTopic));
      } else {
        Serial.print("Failed, rc=");
        Serial.print(mqttclient.state());
        Serial.println(" Retrying in 5 seconds...");
        delay(5000);
      }
      }
    */

    // Route to load style.css file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(LittleFS, "/style.css", "text/css");
    });
    server.on("/principal.css", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(LittleFS, "/principal.css", "text/css");
    });
    //Route to load logo.png file
    server.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(LittleFS, "/logo.png", "image/png");
    });
    // Route for root / web page
    //fazer redirect para /home
    server.on("/home", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(LittleFS, "/index.html", String(), false, processor);
    });
    server.serveStatic("/", LittleFS, "/");
    //----------------------------
    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", String(t).c_str());
      //WiFi.mode(WIFI_STA);
    });
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", String(h).c_str());
    });
    server.on("/ldr", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", String(l).c_str());
    });
    //----------------------------
    server.on("/desligar", HTTP_GET, [](AsyncWebServerRequest * request) {
      //WiFi.mode(WIFI_STA);
      WiFi.softAPdisconnect (true);
      Serial.println("Desligar AP");
      request->send(LittleFS, "/index.html", String(), false, processor);
    });
    //----------------------------
    // Route to set GPIO state to HIGH
    server.on("/13/on", HTTP_GET, [](AsyncWebServerRequest * request) {
      digitalWrite(relePin13, HIGH);
      request->send(LittleFS, "/index.html", String(), false, processor);

    });
    // Route to set GPIO state to LOW
    server.on("/13/off", HTTP_GET, [](AsyncWebServerRequest * request) {
      digitalWrite(relePin13, LOW);
      request->send(LittleFS, "/index.html", String(), false, processor);
    });
    // Route to set GPIO state to HIGH
    server.on("/15/on", HTTP_GET, [](AsyncWebServerRequest * request) {
      digitalWrite(relePin15, HIGH);
      request->send(LittleFS, "/index.html", String(), false, processor);
    });
    // Route to set GPIO state to LOW
    server.on("/15/off", HTTP_GET, [](AsyncWebServerRequest * request) {
      digitalWrite(relePin15, LOW);
      request->send(LittleFS, "/index.html", String(), false, processor);
    });
    server.on("/", HTTP_POST, [](AsyncWebServerRequest * request) {
      int params = request->params();
      for (int i = 0; i < params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isPost()) {
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
          }
          // HTTP POST SERVER IP value
          if (p->name() == PARAM_INPUT_3) {
            serverIp = p->value().c_str();
            Serial.print("SERVER IP set to: ");
            Serial.println(serverIp);
            // Write file to save value
            writeFile(LittleFS, serverIpPath, serverIp.c_str());
          }
          // HTTP POST TOKEN PLT value
          if (p->name() == PARAM_INPUT_4) {
            token = p->value().c_str();
            Serial.print("TOKEN PLT set to: ");
            Serial.println(token);
            // Write file to save value
            writeFile(LittleFS, tokenIpPath, token.c_str());
          }
        }
      }
      // Verificar qual botão foi pressionado
      if (request->hasParam(PARAM_INPUT_1) && request->hasParam(PARAM_INPUT_2)) {
        // Lógica para o Botão 1...
        Serial.println("Botão 1 pressionado");
        Serial.println("Novas Credenciais SSID-" + ssid + " Pass- " + pass);
        // Outras operações para o Botão 1...
      } else if (request->hasParam(PARAM_INPUT_3) && request->hasParam(PARAM_INPUT_4)) {
        // Lógica para o Botão 2...
        Serial.println("Botão 2 pressionado");
        Serial.println("Novos dados ip server " + serverIp + " Token Plt- " + token);
        // Outras operações para o Botão 2...
      }

      request->send(LittleFS, "/index.html", String(), false, processor);
    });

    mqttclient.setServer(mqtt_server, mqtt_port);
    mqttclient.setCallback(callback); // Set the callback function for incoming messages
    mqttclient.subscribe(mqtt_topic); // Subscribe to the MQTT topic

    server.begin();

  }
  else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });

    server.serveStatic("/", LittleFS, "/");

    server.on("/", HTTP_POST, [](AsyncWebServerRequest * request) {
      int params = request->params();
      for (int i = 0; i < params; i++) {
        AsyncWebParameter* p = request->getParam(i);
        if (p->isPost()) {
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
          }
        }
      }
      ESP.restart();
    });
    server.begin();
  }


}

void loop() {

  /* if (!mqttclient.connected()) {
     Serial.println("Lost connection to MQTT broker. Reconnecting...");
     while (!client.connected()) {
       if (mqttclient.connect("ESP8266Client")) {
         Serial.println("Reconnected to MQTT broker");
         mqttclient.subscribe(mqttTopic);
       } else {
         Serial.print("Failed, rc=");
         Serial.print(mqttclient.state());
         Serial.println(" Retrying in 5 seconds...");
         delay(5000);
       }
     }
    }*/



  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisSensor >= intervalSensor) {
    // save the last time you updated the DHT values
    previousMillisSensor = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    //Serial.print("Temperatura---- ");
    //Serial.println(newT);
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      t = newT;
      //Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    //Serial.print("Humidade---- ");
    //Serial.println(newH);
    // if humidity read failed, don't change h value
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      h = newH;
      //Serial.println(h);
    }
    // Read LDR
    float newL = analogRead(LDRPIN);
    //Serial.print("LDR---- ");
    //Serial.println(newL);
    // if ldr read failed, don't change h value
    if (isnan(newL)) {
      Serial.println("Failed to read from LDR sensor!");
    } else {
      l = newL;
      //Serial.println(l);
    }

    unsigned long currentMillis = millis();

    // MQTT handling
    if (currentMillis - previousMillisMqtt >= mqttInterval) {
      previousMillisMqtt = currentMillis;
      if (!mqttclient.connected()) {
        reconnect();
      }
      mqttclient.loop();
      // Your MQTT-related code here
    }

    if (currentMillis - previousMillisHttp >= httpInterval) {
      previousMillisHttp = currentMillis;
      // Non-blocking HTTP requests using HTTPClient
      makeHTTPRequest(newT,newH,newL);

    }



  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  // Handle incoming messages here
  Serial.print("Message: ");
  Serial.write(payload, length);
  Serial.println();
  
  char* message = reinterpret_cast<char*>(payload);
  if (strlen(message) > 0) {
    message[strlen(message) - 1] = '\0'; // Remove the last character
  }
  //-----------
  if (strcmp(message, "1 desligar") == 0) {
    digitalWrite(relePin13, LOW);
    Serial.println("Rele desligado");
  }
  else if (strcmp(message, "1 ligar") == 0) {
    digitalWrite(relePin13, HIGH);
    Serial.println("Rele ligado");
  }
  else if (strcmp(message, "2 desligar") == 0) {
    digitalWrite(relePin15, LOW);
    Serial.println("Rele desligado");
  }
  else if (strcmp(message, "2 ligar") == 0) {
    digitalWrite(relePin15, HIGH);
    Serial.println("Rele ligado");
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttclient.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttclient.connect(clientId.c_str())) {
      Serial.println("Connected to MQTT broker");
      mqttclient.subscribe(mqtt_topic);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttclient.state());
      Serial.println(" Try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void makeHTTPRequest(float newT, float newH,float newL) {
  HTTPClient http;
  http.setTimeout(10000);
  //prepare request
  //----------SENSOR DHT 11 - ID 1 - Temperatura
  String postDataT;
  int idsensorT = 1;
  String medidaT = "C";
  //---
  postDataT = "id_sensor=" + String(idsensorT) + "&valor=" + String(newT) + "&unidade_medida=" + String(medidaT);
  http.begin(client, host);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postDataT);  
  //----------SENSOR DHT 11 - ID 2 - Humidade
  String postDataH;
  int idsensorH = 2;
  String medidaH = "%";
  //---
  postDataH = "id_sensor=" + String(idsensorH) + "&valor=" + String(newH) + "&unidade_medida=" + String(medidaH);
  http.begin(client, host);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postDataH);  
  //----------SENSOR LDR - ID 3 - Luminosidade
  String postDataL;
  int idsensorL = 3;
  String medidaL = "V";
  //---
  postDataL = "id_sensor=" + String(idsensorL) + "&valor=" + String(newL) + "&unidade_medida=" + String(medidaL);
  http.begin(client, host);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postDataL);  
  
  String payload2 = http.getString();
  Serial.println(httpCode);
  Serial.println(payload2);
  http.end();
}
