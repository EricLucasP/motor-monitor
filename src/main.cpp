#include "server.h"
struct Measurement {
  uint32_t time;
  float phase1;
  float phase2;
  float phase3;
};

#define Sensor1 34
#define Sensor2 35
#define Sensor3 36

const int BUFFER_SIZE = 300; // número de medições por vez
Measurement buffer[BUFFER_SIZE];
int bufferIndex = 0;

// Funções
void measure();
void saveDataToFile(Measurement *data, int size);
void taskMeasure(void *pvParameters);
void taskSendData(void *pvParameters);
void serverHandleClient();

uint32_t startTime = 0;
uint32_t totalTime = 0;

void setup() {
  // Serial.begin(115200); // Debug serial
  serverSetup();

  xTaskCreatePinnedToCore(taskMeasure, "measureLoop", 10000 / sizeof(StackType_t), NULL, 1, NULL, 0);
  xTaskCreatePinnedToCore(taskSendData, "DataLoop", 10000 / sizeof(StackType_t), NULL, 1, NULL, 1);
}

void loop() {
}

void taskMeasure(void *pvParameters) {
  while (true) {
    measure();
    delayMicroseconds(10);
  }
}

int test = 0;
void measure() {
  if (test == -1) {
    return;
  }
  if (test == 0) {
    startTime = micros();
  }
  if (test >= 1800 / BUFFER_SIZE) {
    File file = SPIFFS.open("/data.txt", FILE_APPEND);
    totalTime = micros() - startTime;
    file.println(totalTime);
    file.close();
    test = -1;
    return;
  }

  buffer[bufferIndex].phase1 = analogRead(Sensor1);
  buffer[bufferIndex].phase2 = analogRead(Sensor2);
  buffer[bufferIndex].phase3 = analogRead(Sensor3);
  bufferIndex++;

  if (bufferIndex >= BUFFER_SIZE) {
    test++;
    saveDataToFile(buffer, BUFFER_SIZE);
    bufferIndex = 0;
  }
}

void saveDataToFile(Measurement *data, int size) {
  File file = SPIFFS.open("/data.txt", FILE_APPEND);
  if (!file) {
    Serial.println("Erro ao abrir o arquivo para escrita");
    return;
  }

  for (int i = 0; i < size; i++) {
    file.print(buffer[i].time);
    file.print(";");
    file.print(buffer[i].phase1);
    file.print(";");
    file.print(buffer[i].phase2);
    file.print(";");
    file.println(buffer[i].phase3);
  }
  file.close();
}

void taskSendData(void *pvParameters) {
  while (true) {
    serverHandleClient();
    vTaskDelay(1);
  }
}

void serverHandleClient() {
  server.handleClient();
  vTaskDelay(1);
}