#include <ESP8266WiFi.h>
#include "DHT.h"
#include <String.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define DHTPIN 0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

int sensorPinLDR = A0; // define o pino do sensor como A0

// Set web server port number to 80
//ESP8266WebServer server(80);
WiFiServer server(80);

// Variable to store the HTTP request
String header;

void setup() {
  Serial.begin(9600);
  //-----------------------
  // WiFiManager
  WiFiManager wifiManager;
  //wifiManager.resetSettings();
  // fetches ssid and pass from eeprom and tries to connect
  //wifiManager.autoConnect("AP-NAME", "AP-PASSWORD");
  wifiManager.autoConnect("AutoConnectAP");

  // if you get here you have connected to the WiFi
  Serial.println("Connected.");
  //-----------------------
  dht.begin();
  //-----------------------
  server.begin();
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  // Imprime os valores lidos na porta serial
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.print(" °C - Umidade: ");
  Serial.print(humidity);
  Serial.println(" %");

  int sensorValue = analogRead(sensorPinLDR);

  Serial.print(" LDR ");
  Serial.println(sensorValue);

  // Verifica se há clientes conectados
  WiFiClient client = server.available();
  IPAddress ip = WiFi.localIP();

  if (client) {

    // Lê a requisição HTTP
    String request = client.readStringUntil('\r');
    client.flush();
    // Cria a resposta HTTP
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n\r\n";
    response += "<!DOCTYPE html><html lang='en'><head> <meta http-equiv='refresh' content='30'><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>ESP8266 Sensor Data</title><link rel='stylesheet' href='https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css' integrity='sha384-ggOyR0iXCbMQv3Xipma34MD+dH/1fQ784/j6cY/iJTQUOhcWr7x9JvoRxT2MZw1T' crossorigin='anonymous'></head><body>";
    response += "<script>\
                  function refresh(refreshPeriod) \
                  {\
                    setTimeout('location.reload(true)', refreshPeriod);\
                  } \
                  window.onload = refresh(5000);\
                  </script>";
    //-----------------------------
    response += "<div class=\"row\">";
    response += "<div class=\"col-sm-4\">";
    response += "</div>";
    response += "<div class=\"col-sm-4 text-center\">";
    response += "<h5>Configuração ESP8266</h5>";
    response += "<p class=\"text-center\">"+ ip.toString() +"</p>";
    response += "</div>";
    response += "<div class=\"col-sm-4\">";
    response += "</div>";
    response += "</div>";
    //-----------------------------
    response += "<div class=\"row\">";
    response += "<div class=\"col-lg-3\">";
    response += "<div class=\"card mb-3\">";
    response += "<div class=\"card-body\">";
    response += "<h5 class=\"card-title\">Temperature</h5>";
    response += "<h6 class=\"card-subtitle mb-2 text-muted\">" + String(temperature) + "&deg;C Pin: "+ DHTPIN +"</h6>";
    response += "</div></div></div>";
    response += "<div class=\"col-lg-3\">";
    response += "<div class=\"card mb-3\">";
    response += "<div class=\"card-body\">";
    response += "<h5 class=\"card-title\">Humidity</h5>";
    response += "<h6 class=\"card-subtitle mb-2 text-muted\">" + String(humidity) + "% Pin: "+ DHTPIN +"</h6>";
    response += "</div></div></div>";
    response += "<div class=\"col-lg-3\">";
    response += "<div class=\"card mb-3\">";
    response += "<div class=\"card-body\">";
    response += "<h5 class=\"card-title\">LDR</h5>";
    response += "<h6 class=\"card-subtitle mb-2 text-muted\">" + String(sensorValue) + " Pin: "+ sensorPinLDR +"</h6>";
    response += "</div></div></div>";
    response += "<div class=\"col-lg-3\">";
    response += "<div class=\"card mb-3\">";
    response += "<div class=\"card-body\">";
    response += "<h5 class=\"card-title\">Humidade do Solo</h5>";
    response += "<h6 class=\"card-subtitle mb-2 text-muted\"> Será Sensor de Solo</h6>";
    response += "</div></div></div></div>";
    response += "</body></html>";

    // Envia a resposta HTTP para o cliente
    client.print(response);
    delay(1);
    client.stop();
  }

  delay(2000);
}
