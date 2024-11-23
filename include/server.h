#include "secrets.h" // hidden file with secret info (wifi)
#include <Arduino.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <WiFi.h>

// from secrets.h
static const char *ssid = SECRET_SSID;
static const char *password = SECRET_PASS;

void handleRoot();
void handleFileNotFound();
void handleFileRead(String filePath);
String getContentType(String filename);
bool isFile(String path);

WebServer server(80);

void serverSetup() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // web server configuration
  server.on("/", handleRoot);
  server.onNotFound(handleFileNotFound);
  server.begin();
  Serial.println("Web Server started");

  // SPIFFS configuration
  if (!SPIFFS.begin()) {
    Serial.println("Erro ao montar SPIFFS");
    return;
  }
}

void handleFileRead(String filePath) {
  String contentType = getContentType(filePath);

  if (isFile(filePath)) {
    File file = SPIFFS.open(filePath, "r");
    server.streamFile(file, contentType);
    file.close();
  } else {
    server.send(404, "text/plain", "FileNotFound");
  }
}

void handleFileNotFound() {
  String path = server.uri();
  handleFileRead(path);
}

void handleRoot() {
  handleFileRead("/main.html");
}

String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".csv")) {
    return "text/csv";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".json")) {
    return "application/json";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool isFile(String path) {
  File file = SPIFFS.open(path, "r");
  if (!file.isDirectory()) {
    file.close();
    return true;
  }
  Serial.print(path);
  Serial.println(" is Not a File");
  file.close();
  return false;
}
