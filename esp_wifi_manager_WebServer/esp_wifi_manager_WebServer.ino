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
//#include <AsyncTCP.h> so para esp32
#include <LittleFS.h>
#include "FS.h"
//----------------------------

/*const char* ssid = "Vodafone-8B921A";
  const char* password = "8xSGAdbGq5";*/
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
// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

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
// Initialize WiFi
bool initWiFi() {
  if (ssid == "" || ip == "") {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());


  if (!WiFi.config(localIP, localGateway, subnet)) {
    Serial.println("STA Failed to configure");
    return false;
  }
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
  Serial.println(WiFi.localIP());
  return true;
}
//----------------------------
/*const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
   <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <title>ESP8266 Sensor Data</title>
      <link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css' integrity='sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T' crossorigin='anonymous'>
      <style>
         html
         {font-family: Arial; display: inline-block; text-align: center;}
         h2 {font-size: 3.0rem;}
         p {font-size: 3.0rem;}
         .switch {position: relative; display: inline-block; width: 120px; height: 68px}
         .switch input {display: none}
         .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 6px}
         .slider:before {position: absolute; content: ""; height: 25px; width: 25px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 3px}
         input:checked+.slider {background-color: #b30000}
         input:checked+.slider:before {-webkit-transform: translateX(25px); -ms-transform: translateX(25px); transform: translateX(25px)}
         }
      </style>
   </head>
   <script>
      function toggleCheckbox(element) {
      var xhr = new XMLHttpRequest();
      if(element.checked){ xhr.open("GET", "/update?output="+element.id+"&state=1", true); }
      else { xhr.open("GET", "/update?output="+element.id+"&state=0", true); }
      xhr.send();
      }
         setInterval(function ( ) {
           var xhttp = new XMLHttpRequest();
           xhttp.onreadystatechange = function() {
             if (this.readyState == 4 && this.status == 200) {
               document.getElementById("temperature").innerHTML = this.responseText;
             }
           };
           xhttp.open("GET", "/temperature", true);
           xhttp.send();
         }, 2000) ;

         setInterval(function ( ) {
           var xhttp = new XMLHttpRequest();
           xhttp.onreadystatechange = function() {
             if (this.readyState == 4 && this.status == 200) {
               document.getElementById("humidity").innerHTML = this.responseText;
             }
           };
           xhttp.open("GET", "/humidity", true);
           xhttp.send();
         }, 2000 ) ;

         setInterval(function ( ) {
           var xhttp = new XMLHttpRequest();
           xhttp.onreadystatechange = function() {
             if (this.readyState == 4 && this.status == 200) {
               document.getElementById("ldr").innerHTML = this.responseText;
             }
           };
           xhttp.open("GET", "/ldr", true);
           xhttp.send();
         }, 2000 ) ;
   </script>
   <body>
      <h2 class="text-center">ESP Web Server</h2>
      %BUTTONPLACEHOLDER%
      <div class="row">
      <div class="col-lg-3">
         <div class="card mb-3">
            <div class="card-body">
               <h5 class="card-title">Temperature</h5>
               <h6 class="card-subtitle mb-2 text-muted">
                  <span id="temperature">%TEMPERATURE%</span>
                  <sup class="units">&deg;C</sup>
               </h6>
            </div>
         </div>
      </div>
      <div class="col-lg-3">
         <div class="card mb-3">
            <div class="card-body">
               <h5 class="card-title">Humidade</h5>
               <h6 class="card-subtitle mb-2 text-muted">
                  <span id="humidity">%HUMIDITY%</span>
                  <sup class="units">&percnt;</sup>
               </h6>
            </div>
         </div>
      </div>
      <div class="col-lg-3">
         <div class="card mb-3">
            <div class="card-body">
               <h5 class="card-title">LDR</h5>
               <h6 class="card-subtitle mb-2 text-muted">
                  <span id="ldr">%LDR%</span>
                  <sup class="units">v</sup>
               </h6>
            </div>
         </div>
      </div>
   </body>
  </html>)rawliteral";*/
// Replaces placeholder with button section in your web page
String processor(const String& var) {
  Serial.println(var);
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
  return String();
}
void setup() {
  // put your setup code here, to run once:
  // Serial port for debugging purposes
  Serial.begin(9600);
  //----------------------------
  initLittleFS();
  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  gateway = readFile (LittleFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);
  //----------------------------
  dht.begin();
  pinMode(relePin13, OUTPUT);
  digitalWrite(relePin13, LOW);
  pinMode(relePin15, OUTPUT);
  digitalWrite(relePin15, LOW);

  if (initWiFi()) {
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(LittleFS, "/index.html", String(), false, processor);
    });
    server.serveStatic("/", LittleFS, "/");
    // Route to load style.css file
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send(SPIFFS, "/style.css", "text/css");
    });
    //----------------------------
    server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", String(t).c_str());
      //request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
    server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", String(h).c_str());
      //request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
    server.on("/ldr", HTTP_GET, [](AsyncWebServerRequest * request) {
      request->send_P(200, "text/plain", String(l).c_str());
      //request->send(LittleFS, "/index.html", "text/html", false, processor);
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
    //----------------------------
    // Start server
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
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(LittleFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(LittleFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      //delay(3000);
      ESP.restart();
    });
    server.begin();
  }
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisSensor >= intervalSensor) {
    // save the last time you updated the DHT values
    previousMillisSensor = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    Serial.print("Temperatura---- ");
    Serial.println(newT);
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      t = newT;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    Serial.print("Humidade---- ");
    Serial.println(newH);
    // if humidity read failed, don't change h value
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      h = newH;
      Serial.println(h);
    }
    // Read LDR
    float newL = analogRead(LDRPIN);
    Serial.print("LDR---- ");
    Serial.println(newL);
    // if ldr read failed, don't change h value
    if (isnan(newL)) {
      Serial.println("Failed to read from LDR sensor!");
    } else {
      l = newL;
      Serial.println(l);
    }
  }
}
