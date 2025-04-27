#include "scan_utils.h"
#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include "structs.h"
#include "wifi_utils.h"
#include "localization.h"
#include "config.h"

void initHostScan(bool isLocal, String hostsArg, String portsArg) {
  hostCount = 0;
  currentHostIndex = 0;
  scanningHosts = true;

  if (isLocal) {
    // Локальная сеть: сканируем 192.168.x.2-254
    String subnet = WiFi.localIP().toString().substring(0, WiFi.localIP().toString().lastIndexOf('.') + 1);
    for (int i = 2; i <= 254 && hostCount < 30; i++) {
      hosts[hostCount].ip = subnet + String(i);
      hosts[hostCount].isActive = false;
      hosts[hostCount].openPortCount = 0;
      for (int j = 0; j < 20; j++) {
        hosts[hostCount].openPorts[j] = 0;
        hosts[hostCount].services[j] = "";
      }
      hostCount++;
    }
  } else if (hostsArg.indexOf('-') != -1) {
    // Диапазон IP
    int dash = hostsArg.indexOf('-');
    String startIP = hostsArg.substring(0, dash);
    String endIP = hostsArg.substring(dash + 1);
    IPAddress startAddr, endAddr;
    if (!startAddr.fromString(startIP) || !endAddr.fromString(endIP)) {
      Serial.println(loc.getString("INVALID_IP_RANGE"));
      scanningHosts = false;
      return;
    }
    uint32_t start = startAddr;
    uint32_t end = endAddr;
    for (uint32_t ip = start; ip <= end && hostCount < 30; ip++) {
      IPAddress addr(ip);
      hosts[hostCount].ip = addr.toString();
      hosts[hostCount].isActive = false;
      hosts[hostCount].openPortCount = 0;
      for (int j = 0; j < 20; j++) {
        hosts[hostCount].openPorts[j] = 0;
        hosts[hostCount].services[j] = "";
      }
      hostCount++;
    }
  } else if (hostsArg.indexOf(',') != -1) {
    // Список IP
    String ipList = hostsArg;
    int start = 0;
    int comma = ipList.indexOf(',');
    while (comma != -1 && hostCount < 30) {
      String ip = ipList.substring(start, comma);
      if (isIPAddress(ip)) {
        hosts[hostCount].ip = ip;
        hosts[hostCount].isActive = false;
        hosts[hostCount].openPortCount = 0;
        for (int j = 0; j < 20; j++) {
          hosts[hostCount].openPorts[j] = 0;
          hosts[hostCount].services[j] = "";
        }
        hostCount++;
      }
      start = comma + 1;
      comma = ipList.indexOf(',', start);
    }
    String ip = ipList.substring(start);
    if (isIPAddress(ip) && hostCount < 30) {
      hosts[hostCount].ip = ip;
      hosts[hostCount].isActive = false;
      hosts[hostCount].openPortCount = 0;
      for (int j = 0; j < 20; j++) {
        hosts[hostCount].openPorts[j] = 0;
        hosts[hostCount].services[j] = "";
      }
      hostCount++;
    }
  } else {
    // Одиночный IP или URL
    String ip = resolveHost(hostsArg); // Разрешаем URL в IP
    if (isIPAddress(ip) && hostCount < 10) {
      hosts[hostCount].ip = ip;
      hosts[hostCount].isActive = false;
      hosts[hostCount].openPortCount = 0;
      for (int j = 0; j < 20; j++) {
        hosts[hostCount].openPorts[j] = 0;
        hosts[hostCount].services[j] = "";
      }
      hostCount++;
    }
  }

  if (hostCount == 0) {
    Serial.println(loc.getString("NO_HOSTS"));
    scanningHosts = false;
  }
}

void scanHosts() {
  if (currentHostIndex >= hostCount) {
    scanningHosts = false;
    scanningPorts = (countActiveHosts() > 0);
    currentHostIndex = 0;
    currentPortIndex = 0;
    return;
  }

  String ip = hosts[currentHostIndex].ip;
  if (!silentMode) {
    Serial.println(loc.getString("PINGING", ip));
  }

  if (Ping.ping(ip.c_str(), 1)) {
    hosts[currentHostIndex].isActive = true;
    if (!silentMode) {
      Serial.println(loc.getString("HOST_ACTIVE"));
    }
  } else {
    hosts[currentHostIndex].isActive = false;
    if (!silentMode) {
      Serial.println(loc.getString("HOST_INACTIVE"));
    }
  }

  currentHostIndex++;
  currentStep++;
  progress = (float)currentStep / totalSteps * 100.0;
  if (!silentMode) {
    Serial.println(loc.getString("PROGRESS", progress));
  }
}

String scanService(String ip, int port) {
  WiFiClient client;
  String service = "Unknown";

  if (client.connect(ip.c_str(), port)) {
    // Даём серверу 1 секунду на отправку баннера
    unsigned long timeout = millis() + 1000;
    while (!client.available() && millis() < timeout) {
      delay(10);
    }

    if (client.available()) {
      String banner = client.readStringUntil('\n');
      banner.trim();
      if (banner.length() > 0) {
        if (banner.startsWith("HTTP")) {
          service = "HTTP";
          // Отправляем простой GET-запрос для получения заголовка Server
          client.print("GET / HTTP/1.0\r\n\r\n");
          timeout = millis() + 1000;
          while (!client.available() && millis() < timeout) {
            delay(10);
          }
          while (client.available()) {
            String line = client.readStringUntil('\n');
            if (line.startsWith("Server: ")) {
              service = "HTTP (" + line.substring(8) + ")";
              break;
            }
          }
        } else if (banner.startsWith("SSH")) {
          service = "SSH (" + banner + ")";
        } else if (banner.startsWith("220") && port == 21) {
          service = "FTP (" + banner.substring(4) + ")";
        } else if (banner.startsWith("220") && port == 25) {
          service = "SMTP (" + banner.substring(4) + ")";
        } else if (port == 23) {
          service = "Telnet";
        } else {
          service = "Unknown (" + banner + ")";
        }
      }
    }
    client.stop();
  } else {
    if (debugMode) {
      Serial.println("[DEBUG] Failed to connect to " + ip + ":" + String(port));
    }
  }

  return service;
}

void scanPorts() {
  if (currentHostIndex >= hostCount) {
    scanningPorts = false;
    if (!silentMode) {
      for (int i = 0; i < hostCount; i++) {
        if (hosts[i].isActive) {
          Serial.println(loc.getString("HOST_RESULT", hosts[i].ip));
          for (int j = 0; j < hosts[i].openPortCount; j++) {
            Serial.println(loc.getString("PORT_OPEN", String(hosts[i].openPorts[j]), hosts[i].services[j]));
          }
        }
      }
    }
    if (uploadReport) {
      Serial.println(loc.getString("UPLOAD_PROMPT"));
    }
    return;
  }

  if (!hosts[currentHostIndex].isActive) {
    currentHostIndex++;
    return;
  }

  if (scanMode == "range") {
    if (currentPort < startPort || currentPort > endPort) {
      currentPort = startPort;
      currentPortIndex = 0;
      currentHostIndex++;
      return;
    }
  } else {
    if (currentPortIndex >= portListCount) {
      currentPortIndex = 0;
      currentHostIndex++;
      return;
    }
    currentPort = portList[currentPortIndex];
  }

  String ip = hosts[currentHostIndex].ip;
  WiFiClient client;
  if (client.connect(ip.c_str(), currentPort)) {
    if (hosts[currentHostIndex].openPortCount < 20) {
      hosts[currentHostIndex].openPorts[hosts[currentHostIndex].openPortCount] = currentPort;
      hosts[currentHostIndex].services[hosts[currentHostIndex].openPortCount] = scanService(ip, currentPort);
      hosts[currentHostIndex].openPortCount++;
    }
    if (!silentMode) {
      Serial.println(loc.getString("PORT_OPEN", String(currentPort), hosts[currentHostIndex].services[hosts[currentHostIndex].openPortCount - 1]));
    }
    client.stop();
  } else {
    if (debugMode) {
      Serial.println("[DEBUG] Port " + String(currentPort) + " on " + ip + " is closed");
    }
  }

  if (scanMode == "range") {
    currentPort++;
  } else {
    currentPortIndex++;
  }

  currentStep++;
  progress = (float)currentStep / totalSteps * 100.0;
  if (!silentMode) {
    Serial.println(loc.getString("PROGRESS", String(progress)));
  }
}

void manualPing(String host) {
  String ip = resolveHost(host);
  if (Ping.ping(ip.c_str(), 1)) {
    Serial.println(loc.getString("PING_SUCCESS", host, ip));
  } else {
    Serial.println(loc.getString("PING_FAILED", host));
  }
}

int countActiveHosts() {
  int active = 0;
  for (int i = 0; i < hostCount; i++) {
    if (hosts[i].isActive) {
      active++;
    }
  }
  return active;
}

void clearMemory() {
  for (int i = 0; i < hostCount; i++) {
    hosts[i].ip = "";
    hosts[i].isActive = false;
    hosts[i].openPortCount = 0;
    for (int j = 0; j < 20; j++) {
      hosts[i].openPorts[j] = 0;
      hosts[i].services[j] = "";
    }
  }
  hostCount = 0;
  currentHostIndex = 0;
  currentPortIndex = 0;
  currentPort = 0;
}