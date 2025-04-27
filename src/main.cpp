#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <LittleFS.h>
#include "wifi_utils.h"
#include "scan_utils.h"
#include "http_utils.h"
#include "command_utils.h"
#include "structs.h"
#include "localization.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  loc.init();
  if (!LittleFS.begin()) {
    Serial.println(F("Failed to mount LittleFS"));
  }
  Serial.println(loc.getString("HELP_PROMPT"));
  scanWiFiNetworks();
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    
    if (awaitingNetworkSelection) {
      processWiFiSelection(input); // Обрабатываем выбор сети
    } else if (awaitingPassword) {
      if (selectedPassword == "") {
        selectedPassword = input;
        connectToWiFi();
      } else {
        processDeviceCode(input); // Обрабатываем код для netscout.tech
      }
    } else if (input == "help") {
      printHelp();
    } else if (input.startsWith("scan ") || input.startsWith("ping ") || input.startsWith("lang ")) {
      parseCommand(input);
    } else if (input == "stop") {
      stopScan();
    } else if (input.equalsIgnoreCase("yes") || input.equalsIgnoreCase("no")) {
      processUploadDecision(input);
    } else {
      Serial.println(loc.getString("UNKNOWN_COMMAND"));
    }
  }
  
  if (scanningHosts) {
    scanHosts();
  } else if (scanningPorts) {
    scanPorts();
  }
}