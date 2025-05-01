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
    Serial.flush();
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
    Serial.println("[DEBUG] URL: https://neoscout.ru/upload_report");
    Serial.println("[DEBUG] Headers: Content-Type: application/json");
    Serial.println("[DEBUG] Body: " + jsonString);
    Serial.flush();
  }

  http.begin(client, String(BASE_URL) + "/upload_report");
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(jsonString);
  
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.printf("Report uploaded: %s\n", response.c_str());
    if (debugMode) {
      Serial.println("[DEBUG] HTTP response code: " + String(httpCode));
      Serial.println("[DEBUG] Response body: " + response);
      Serial.flush();
    }
  } else {
    String response = http.getString();
    Serial.printf("Upload failed. Code: %d\n", httpCode);
    Serial.println(F("Server response: ") + (response == "" ? "No response" : response));
    if (debugMode) {
      Serial.println("[DEBUG] HTTP response code: " + String(httpCode));
      Serial.println("[DEBUG] Response body: " + response);
      Serial.flush();
    }
  }
  
  http.end();
}

String getExternalIP() {
  WiFiClient client;
  HTTPClient http;
  String ip = "";
  
  http.begin(client, "http://api.ipify.org");
  int httpCode = http.GET();
  
  if (httpCode == HTTP_CODE_OK) {
    ip = http.getString();
  } else {
    if (debugMode) {
      Serial.println("[DEBUG] HTTP error: " + String(httpCode));
    }
    Serial.println("Failed to get external IP");
  }
  
  http.end();
  return ip;
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
  if (lastHostsArg == "all" || lastHostsArg.indexOf('-') != -1) {
    return getExternalIP();
  }

  bool allLocal = true;
  if (lastHostsArg.indexOf(',') != -1) {
    String ipList = lastHostsArg;
    int start = 0;
    int comma = ipList.indexOf(',');
    while (comma != -1) {
      String ip = ipList.substring(start, comma);
      if (!isLocalIP(ip)) {
        allLocal = false;
        break;
      }
      start = comma + 1;
      comma = ipList.indexOf(',', start);
    }
    String ip = ipList.substring(start);
    if (!isLocalIP(ip)) {
      allLocal = false;
    }
  } else {
    allLocal = isLocalIP(lastHostsArg);
  }

  if (allLocal) {
    return getExternalIP();
  }

  String firstHost = lastHostsArg;
  if (lastHostsArg.indexOf(',') != -1) {
    firstHost = lastHostsArg.substring(0, lastHostsArg.indexOf(','));
  }
  return resolveHost(firstHost);
}