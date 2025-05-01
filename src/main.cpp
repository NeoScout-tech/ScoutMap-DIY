#include <Arduino.h>
#include <ESP8266WiFi.h>
// #include <FS.h>
// #include <LittleFS.h>
#include <ArduinoJson.h>
#include "command_utils.h"
#include "scan_utils.h"
#include "wifi_utils.h"
#include "http_utils.h"
#include "structs.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    // if (!LittleFS.begin()) {
    //     Serial.println("Failed to mount LittleFS");
    // }
    Serial.println("Enter 'help' for commands.");
    scanWiFiNetworks();
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        if (awaitingNetworkSelection) {
            processWiFiSelection(input);
        } else if (awaitingPassword) {
            if (selectedPassword == "") {
                if (browserMode) {
                    StaticJsonDocument<256> doc;
                    DeserializationError error = deserializeJson(doc, input);
                    if (error || !doc.containsKey("password")) {
                        Serial.println("{\"status\":\"error\",\"message\":\"Invalid JSON or missing password\"}");
                        return;
                    }
                    selectedPassword = doc["password"].as<String>();
                } else {
                    selectedPassword = input;
                }
                connectToWiFi();
            } else {
                processDeviceCode(input);
            }
        } else if (uploadReport) {
            processUploadDecision(input);
        } else {
            parseCommand(input);
        }
    }
    if (scanningHosts) {
        scanHosts();
    } else if (scanningPorts) {
        scanPorts();
    }
    yield();
}