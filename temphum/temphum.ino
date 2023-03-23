#include <ESP8266WiFi.h>
#include "DHT.h"
#include <WiFiUdp.h>
#include <coap-simple.h>

#define DHTPIN 0
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Vodafone-C3F424";
const char* password = "CZVTPG2zAQ";
//------------------------

//-------------------------
void setup() {
  Serial.begin(9600);
  //-----------------------

  //Conecta-se à rede WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando à rede WiFi...");
  }
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  //-----------------------
  dht.begin();
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
  //-----------------
  int sensorValue = analogRead(A0);              // read the input on analog pin 0
  float voltage = sensorValue * (5.0 / 1023.0);  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V)
  Serial.print(" LDR ");
  Serial.println(voltage);  // print out the value you read
  //--------------
  delay(2000);
}
