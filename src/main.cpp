#include "ADS1X15.h"
#include "secrets.h"
#include <WebServer.h>
#include <WiFi.h>

// from secrets.h
static const char *ssid = ssidHide;
static const char *password = passwordHide;

ADS1115 ADS(0x48);
WebServer server(80);

const int buffer_size = 860; // 860 amostras por segundo
float buffer[buffer_size];
uint8_t buffer_index = 0;

uint32_t lastSendTime = 0;
uint32_t sendInterval = 1000; // 1 segundo

// Funções
void setup();
void loop();
void handleRoot();
void handleData();
void sendData();

void setup() {
  Serial.begin(250000); // Aumentar baud rate
  delay(100);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Configuração do ADS1115
  Wire.begin();
  Wire.setClock(400000);
  ADS.begin();
  ADS.setGain(0);
  ADS.setDataRate(7);

  // Configuração do servidor web
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("Web Server started");

  lastSendTime = millis();
}

void loop() {
  server.handleClient();

  // Coleta de dados do ADS1115
  if (!ADS.isBusy()) {
    buffer[buffer_index++] = ADS.getValue() * ADS.toVoltage();
    if (buffer_index >= buffer_size)
      buffer_index = 0;
    ADS.requestADC(0);
  }

  // Envio de dados a cada 1 segundo
  if (millis() - lastSendTime >= sendInterval) {
    sendData();
    lastSendTime = millis();
  }
}

void handleRoot() {
  server.send(200, "text/html", "<html><body><h1>ESP32 Data</h1><div id='data'></div><script>setInterval(() => fetch('/data').then(res => res.json()).then(data => document.getElementById('data').innerHTML = data.join('<br>')), 1000);</script></body></html>");
}

void handleData() {
  String data = "[";
  for (int i = 0; i < buffer_size; i++) {
    if (i > 0)
      data += ",";
    data += String(buffer[i], 3);
  }
  data += "]";
  server.send(200, "application/json", data);
}

void sendData() {
  handleData(); // Função para enviar dados ao cliente
}
