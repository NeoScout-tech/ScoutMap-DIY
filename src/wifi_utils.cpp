#include "wifi_utils.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <FS.h>
#include "structs.h"
#include "localization.h"
#include <LittleFS.h>
#include <WiFiClientSecure.h>

void scanWiFiNetworks() {
  Serial.println(loc.getString("SCANNING_WIFI"));
  WiFi.disconnect();
  delay(100);
  
  networkCount = WiFi.scanNetworks(); // Сохраняем число сетей
  if (networkCount == 0) {
    Serial.println(loc.getString("NETWORKS_NOT_FOUND"));
    delay(5000);
    scanWiFiNetworks();
    return;
  }
  
  Serial.println(loc.getString("FOUND_NETWORKS", networkCount));
  for (int i = 0; i < networkCount; i++) {
    Serial.println(loc.getString("NETWORK_ITEM", i + 1, WiFi.SSID(i), WiFi.RSSI(i)));
  }
  
  Serial.println(loc.getString("ENTER_WIFI_NUMBER", networkCount));
  awaitingNetworkSelection = true;
}

void processWiFiSelection(String input) {
  int networkNumber = input.toInt();
  
  if (networkNumber < 1 || networkNumber > networkCount) { // Используем networkCount
    Serial.println(loc.getString("INVALID_NUMBER", networkCount));
    awaitingNetworkSelection = false;
    scanWiFiNetworks();
    return;
  }
  
  selectedSSID = WiFi.SSID(networkNumber - 1);
  Serial.println(loc.getString("SELECTED_SSID", selectedSSID));
  Serial.println(loc.getString("ENTER_PASSWORD", selectedSSID));
  awaitingNetworkSelection = false;
  awaitingPassword = true;
}

void connectToWiFi() {
  WiFi.begin(selectedSSID.c_str(), selectedPassword.c_str());
  Serial.println(loc.getString("CONNECTING_TO", selectedSSID));
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println(loc.getString("CONNECTED", WiFi.localIP().toString()));
    if (hasInternet()) {
      promptDeviceConnection();
    } else {
      Serial.println(F("No internet connection. Skipping device connection."));
    }
  } else {
    Serial.println(loc.getString("CONNECTION_FAILED"));
    selectedSSID = "";
    selectedPassword = "";
    awaitingPassword = false;
    awaitingNetworkSelection = false;
    scanWiFiNetworks();
  }
}

bool isLocalIP(String ip) {
  IPAddress addr;
  if (!addr.fromString(ip)) {
    return false;
  }
  uint32_t ipAddr = (uint32_t)addr;
  return (ipAddr & 0xFFFF0000) == 0xC0A80000 || // 192.168.x.x
         (ipAddr & 0xFF000000) == 0x0A000000 || // 10.x.x.x
         (ipAddr & 0xFFF00000) == 0xAC100000;   // 172.16.x.x - 172.31.x.x
}

bool isIPAddress(String str) {
  IPAddress addr;
  return addr.fromString(str);
}

bool hasInternet() {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, "http://api.ipify.org");
  int httpCode = http.GET();
  bool result = (httpCode == HTTP_CODE_OK);
  http.end();
  return result;
}

void promptDeviceConnection() {
  if (LittleFS.begin()) {
    File file = LittleFS.open("/api_key.txt", "r");
    if (file) {
      apiKey = file.readString();
      file.close();
      deviceConnected = true;
      Serial.println(F("Device already connected with stored API key."));
      uploadReport = true;
      awaitingPassword = false;
      return;
    }
  }

  Serial.println(F("Internet detected. Would you like to connect this device to netscout.tech? (yes/no)"));
  while (true) {
    if (Serial.available()) {
      String input = Serial.readStringUntil('\n');
      input.trim();
      if (input.equalsIgnoreCase("yes")) {
        Serial.println(F("Please visit https://netscout.tech/dashboard?menu=devices, click 'Connect New Device', and enter the provided code."));
        Serial.println(F("When ready, enter the code and press 'y':"));
        awaitingPassword = true;
        break;
      } else if (input.equalsIgnoreCase("no")) {
        uploadReport = false;
        Serial.println(F("Device connection skipped. Reports will not be uploaded."));
        awaitingPassword = false; // Сбрасываем флаг
        break;
      } else {
        Serial.println(loc.getString("INVALID_UPLOAD_INPUT"));
      }
    }
    delay(100);
  }
}

void processDeviceCode(String input) {
  if (input.equalsIgnoreCase("y")) {
    Serial.println(F("Enter the 6-digit code from netscout.tech:"));
    return;
  }

  if (input.length() != 6 || !input.toInt()) {
    Serial.println(F("Invalid code. Please enter a 6-digit code:"));
    return;
  }

  WiFiClientSecure client;
  HTTPClient http;
  client.setInsecure(); // Игнорируем проверку SSL сертификата
  http.begin(client, "https://netscout.tech/connect_device");
  http.addHeader("Content-Type", "application/json");

  DynamicJsonDocument doc(1024); // Увеличиваем размер буфера
  doc["code"] = input;
  JsonObject deviceInfo = doc.createNestedObject("device_info");
  deviceInfo["name"] = "NetScout DIY (ESP8266)";
  deviceInfo["serial_number"] = String(ESP.getChipId(), HEX);

  String jsonString;
  serializeJson(doc, jsonString);

  int httpCode = http.POST(jsonString);
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    DynamicJsonDocument respDoc(256);
    DeserializationError error = deserializeJson(respDoc, response);
    if (!error && respDoc.containsKey("api_key")) {
      apiKey = respDoc["api_key"].as<String>();
      if (LittleFS.begin()) {
        File file = LittleFS.open("/api_key.txt", "w");
        if (file) {
          file.print(apiKey);
          file.close();
          deviceConnected = true;
          uploadReport = true;
          Serial.println(F("Device connected successfully! API key saved."));
          awaitingPassword = false;
        }
      }
    } else {
      Serial.println(F("Failed to parse response or no API key received."));
    }
  } else {
    Serial.println(F("Connection failed. HTTP code: ") + String(httpCode));
  }
  http.end();
  awaitingPassword = false;
}