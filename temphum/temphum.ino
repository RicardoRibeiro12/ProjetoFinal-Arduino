#include <ESP8266WiFi.h>
#include "DHT.h"

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#define DHTPIN 0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

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

  int sensorValue = analogRead(A0);

  Serial.print(" LDR ");
  Serial.println(sensorValue);
  
  // Verifica se há clientes conectados
  WiFiClient client = server.available();
  if (client) {
    // Lê a requisição HTTP
    String request = client.readStringUntil('\r');
    client.flush();

    // Cria a resposta HTTP
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n\r\n";
    response += "<html><body>";
    response += "Valor da Temperatura: " + String(temperature) + "<br>";
    response += "Valor da Humidade: " + String(humidity) + "<br>";
    response += "Valor da LDR: " + String(sensorValue) + "<br>";
    response += "</body></html>";

    // Envia a resposta HTTP para o cliente
    client.print(response);
    delay(1);
    client.stop();
  }

  delay(2000);
}
