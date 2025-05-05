#include "http_utils.h"
#include <ArduinoJson.h>

void uploadScanReport() {
    if (WiFi.status() != WL_CONNECTED || !deviceConnected || apiKey == "") {
        Serial.println("Connection failed");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    if (debugMode) {
        Serial.println("[DEBUG] Starting JSON formation...");
    }

    StaticJsonDocument<2048> doc;
    doc["api_key"] = apiKey;
    doc["location"] = getLocation();

    JsonArray hostsArray = doc.createNestedArray("hosts");

    for (int i = 0; i < hostCount; i++) {
        JsonObject host = hostsArray.createNestedObject();
        host["ip"] = hosts[i].ip;
        host["status"] = hosts[i].isActive ? "online" : "offline";

        JsonArray portsArray = host.createNestedArray("ports");
        JsonArray servicesArray = host.createNestedArray("services");
        for (int j = 0; j < hosts[i].openPortCount; j++) {
            portsArray.add(hosts[i].openPorts[j]);
            servicesArray.add(hosts[i].services[j]);
        }
    }

    String jsonString;
    serializeJson(doc, jsonString);

    if (debugMode) {
        Serial.println("[DEBUG] HTTP request:");
        Serial.println("[DEBUG] Method: POST");
        Serial.println("[DEBUG] URL: " + String(BASE_URL) + "/upload");
        Serial.println("[DEBUG] Headers: Content-Type: application/json");
        Serial.println("[DEBUG] Body: " + jsonString);
    }

    http.begin(client, String(BASE_URL) + "/upload");
    http.addHeader("Content-Type", "application/json");

    int httpCode = http.POST(jsonString);

    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        Serial.printf("Report uploaded: %s\n", response.c_str());
        if (debugMode) {
            Serial.println("[DEBUG] HTTP response code: " + String(httpCode));
            Serial.println("[DEBUG] Response body: " + response);
        }
    } else {
        String response = http.getString();
        Serial.print("Server response: ");
        Serial.println(response == "" ? "No response" : response);
        Serial.printf("Upload failed. Code: %d\n", httpCode);
        if (debugMode) {
            Serial.println("[DEBUG] HTTP response code: " + String(httpCode));
            Serial.println("[DEBUG] Response body: " + response);
        }
    }

    http.end();
}

String resolveHost(String host) {
    if (isIPAddress(host)) {
        return host;
    }

    if (debugMode) {
        Serial.println("[DEBUG] Resolving host: " + host);
    }

    IPAddress ip;
    if (WiFi.hostByName(host.c_str(), ip)) {
        return ip.toString();
    } else {
        if (debugMode) {
            Serial.println("[DEBUG] Failed to resolve host: " + host);
        }
        Serial.printf("DNS failed for %s\n", host.c_str());
        return "";
    }
}

String getLocation() {
    if (locationName == "") {
        char deviceName[32];
        WiFi.macAddress().toCharArray(deviceName, 32);
        char timestamp[20];
        time_t now = time(nullptr);
        strftime(timestamp, sizeof(timestamp), "%Y%m%d", localtime(&now));
        locationName = String(deviceName) + "-ScoutMap-" + String(timestamp);
    }
    return locationName;
}