#include "http_utils.h"
#include <Arduino.h> // Для Serial

void uploadScanReport() {
  if (WiFi.status() != WL_CONNECTED || !deviceConnected || apiKey == "") {
    Serial.println(loc.getString("CONNECTION_FAILED"));
    return;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  
  if (debugMode) {
    Serial.println("[DEBUG] Starting JSON formation...");
    Serial.flush();
  }

  DynamicJsonDocument doc(2048);
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
    Serial.println("[DEBUG] URL: https://netscout.tech/upload_report");
    Serial.println("[DEBUG] Headers: Content-Type: application/json");
    Serial.println("[DEBUG] Body: " + jsonString);
    Serial.flush();
  }

  http.begin(client, "https://netscout.tech/upload_report");
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(jsonString);
  
  if (httpCode == HTTP_CODE_OK) {
    String response = http.getString();
    Serial.println(loc.getString("REPORT_UPLOADED", response));
    if (debugMode) {
      Serial.println("[DEBUG] HTTP response code: " + String(httpCode));
      Serial.println("[DEBUG] Response body: " + response);
      Serial.flush();
    }
  } else {
    String response = http.getString();
    Serial.println(loc.getString("REPORT_UPLOAD_FAILED", String(httpCode)));
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
    Serial.println(loc.getString("EXTERNAL_IP_FAILED"));
  }
  
  http.end();
  return ip;
}

String resolveHost(String host) {
  if (isIPAddress(host)) {
    return host; // Если это уже IP, возвращаем без изменений
  }

  if (debugMode) {
    Serial.println("[DEBUG] Resolving host: " + host);
  }

  IPAddress ip;
  if (WiFi.hostByName(host.c_str(), ip)) {
    return ip.toString(); // Преобразуем IPAddress в String
  } else {
    if (debugMode) {
      Serial.println("[DEBUG] Failed to resolve host: " + host);
    }
    Serial.println(loc.getString("DNS_FAILED", host));
    return host; // Возвращаем исходный хост в случае неудачи
  }
}

String getLocation() {
  if (lastHostsArg == "all" || lastHostsArg.indexOf('-') != -1) {
    return getExternalIP(); // Локальная сеть — внешний IP
  }

  bool allLocal = true;
  if (lastHostsArg.indexOf(',') != -1) {
    // Список IP
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
    // Одиночный IP или URL
    allLocal = isLocalIP(lastHostsArg);
  }

  if (allLocal) {
    return getExternalIP(); // Локальная сеть — внешний IP
  }

  // Удалённый хост — первый IP или разрешённый URL
  String firstHost = lastHostsArg;
  if (lastHostsArg.indexOf(',') != -1) {
    firstHost = lastHostsArg.substring(0, lastHostsArg.indexOf(','));
  }
  return resolveHost(firstHost); // Разрешаем URL в IP, если нужно
}