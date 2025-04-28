#include "scan_utils.h"
#include <WiFi.h>
#include <ESP32Ping.h>
#include "structs.h"
#include "wifi_utils.h"
#include "config.h"
#include <ArduinoJson.h>

void initHostScan(bool isLocal, String hostsArg, String portsArg) {
    hostCount = 0;
    currentHostIndex = 0;
    scanningHosts = true;

    if (isLocal) {
        String subnet = WiFi.localIP().toString().substring(0, WiFi.localIP().toString().lastIndexOf('.') + 1);
        for (int i = 2; i <= 254 && hostCount < 256; i++) {
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
        int dash = hostsArg.indexOf('-');
        String startIP = hostsArg.substring(0, dash);
        String endIP = hostsArg.substring(dash + 1);
        IPAddress startAddr, endAddr;
        if (!startAddr.fromString(startIP) || !endAddr.fromString(endIP)) {
            if (browserMode) {
                Serial.println("{\"status\":\"error\",\"message\":\"Invalid IP range\"}");
            } else {
                Serial.println("Invalid IP range");
            }
            scanningHosts = false;
            return;
        }
        uint32_t start = startAddr;
        uint32_t end = endAddr;
        for (uint32_t ip = start; ip <= end && hostCount < 256; ip++) {
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
        String ipList = hostsArg;
        int start = 0;
        int comma = ipList.indexOf(',');
        while (comma != -1 && hostCount < 256) {
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
        if (isIPAddress(ip) && hostCount < 256) {
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
        String ip = resolveHost(hostsArg);
        if (isIPAddress(ip) && hostCount < 256) {
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
        if (browserMode) {
            Serial.println("{\"status\":\"error\",\"message\":\"No hosts to scan\"}");
        } else {
            Serial.println("No hosts to scan");
        }
        scanningHosts = false;
    }
}

void scanHosts() {
    if (currentHostIndex >= hostCount) {
        scanningHosts = false;
        int activeHosts = countActiveHosts();
        if (activeHosts > 0) {
            scanningPorts = true;
            if (scanMode == "range") {
                totalSteps = hostCount + activeHosts * (endPort - startPort + 1);
            } else {
                totalSteps = hostCount + activeHosts * portListCount;
            }
        } else {
            totalSteps = hostCount;
        }
        currentHostIndex = 0;
        currentPortIndex = 0;
        if (browserMode) {
            StaticJsonDocument<2048> doc;
            doc["type"] = "host_scan_results";
            JsonArray hostsArray = doc.createNestedArray("hosts");
            for (int i = 0; i < hostCount; i++) {
                JsonObject host = hostsArray.createNestedObject();
                host["ip"] = hosts[i].ip;
                host["status"] = hosts[i].isActive ? "online" : "offline";
            }
            String jsonString;
            serializeJson(doc, jsonString);
            Serial.println(jsonString);
        } else if (!silentMode) {
            Serial.println("Host scan results:");
            for (int i = 0; i < hostCount; i++) {
                Serial.printf("Host %s: %s\n", hosts[i].ip.c_str(), hosts[i].isActive ? "online" : "offline");
            }
        }
        return;
    }

    String ip = hosts[currentHostIndex].ip;
    if (!silentMode && !browserMode) {
        Serial.printf("Pinging %s...\n", ip.c_str());
    }

    if (Ping.ping(ip.c_str(), 1)) {
        hosts[currentHostIndex].isActive = true;
        if (!silentMode && !browserMode) {
            Serial.printf("Host %s is online\n", ip.c_str());
        }
    } else {
        hosts[currentHostIndex].isActive = false;
        if (!silentMode && !browserMode) {
            Serial.printf("Host %s is offline\n", ip.c_str());
        }
    }

    currentHostIndex++;
    currentStep++;
    progress = min((float)currentStep / totalSteps * 100.0, 100.0);
    if (browserMode) {
        StaticJsonDocument<256> doc;
        doc["type"] = "progress";
        doc["value"] = progress;
        String jsonString;
        serializeJson(doc, jsonString);
        Serial.println(jsonString);
    } else if (!silentMode) {
        Serial.printf("Progress: %.1f%%\n", progress);
    }
}

String scanService(String ip, int port) {
    WiFiClient client;
    String service = "Unknown";

    if (client.connect(ip.c_str(), port)) {
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
    } else if (debugMode) {
        Serial.println("[DEBUG] Failed to connect to " + ip + ":" + String(port));
    }

    return service;
}

void scanPorts() {
    if (currentHostIndex >= hostCount) {
        scanningPorts = false;
        if (browserMode) {
            StaticJsonDocument<2048> doc;
            doc["type"] = "port_scan_results";
            JsonArray hostsArray = doc.createNestedArray("hosts");
            for (int i = 0; i < hostCount; i++) {
                if (hosts[i].isActive) {
                    JsonObject host = hostsArray.createNestedObject();
                    host["ip"] = hosts[i].ip;
                    JsonArray portsArray = host.createNestedArray("ports");
                    JsonArray servicesArray = host.createNestedArray("services");
                    for (int j = 0; j < hosts[i].openPortCount; j++) {
                        portsArray.add(hosts[i].openPorts[j]);
                        servicesArray.add(hosts[i].services[j]);
                    }
                }
            }
            String jsonString;
            serializeJson(doc, jsonString);
            Serial.println(jsonString);
        } else if (!silentMode) {
            for (int i = 0; i < hostCount; i++) {
                if (hosts[i].isActive) {
                    Serial.printf("Host %s:\n", hosts[i].ip.c_str());
                    for (int j = 0; j < hosts[i].openPortCount; j++) {
                        Serial.printf("Port %d is open (%s)\n", hosts[i].openPorts[j], hosts[i].services[j].c_str());
                    }
                }
            }
        }
        if (uploadReport) {
            if (browserMode) {
                Serial.println("{\"type\":\"prompt\",\"message\":\"Upload report to server? (yes/no)\"}");
            } else {
                Serial.println("Upload report to server? (yes/no)");
            }
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
        if (!silentMode && !browserMode) {
            Serial.printf("Port %d is open (%s)\n", currentPort, hosts[currentHostIndex].services[hosts[currentHostIndex].openPortCount - 1].c_str());
        }
        client.stop();
    } else {
        if (errno == 104 && hosts[currentHostIndex].openPortCount < 20) {
            hosts[currentHostIndex].openPorts[hosts[currentHostIndex].openPortCount] = currentPort;
            hosts[currentHostIndex].services[hosts[currentHostIndex].openPortCount] = "Reset by peer";
            hosts[currentHostIndex].openPortCount++;
            if (!silentMode && !browserMode) {
                Serial.printf("Port %d is open (Reset by peer)\n", currentPort);
            }
        } else if (debugMode) {
            Serial.println("[DEBUG] Port " + String(currentPort) + " on " + ip + " is closed");
        }
    }

    if (scanMode == "range") {
        currentPort++;
    } else {
        currentPortIndex++;
    }

    currentStep++;
    progress = min((float)currentStep / totalSteps * 100.0, 100.0);
    if (browserMode) {
        StaticJsonDocument<256> doc;
        doc["type"] = "progress";
        doc["value"] = progress;
        String jsonString;
        serializeJson(doc, jsonString);
        Serial.println(jsonString);
    } else if (!silentMode) {
        Serial.printf("Progress: %.1f%%\n", progress);
    }
}

void manualPing(String host) {
    String ip = resolveHost(host);
    if (browserMode) {
        StaticJsonDocument<256> doc;
        doc["type"] = "ping_result";
        doc["host"] = host;
        if (Ping.ping(ip.c_str(), 1)) {
            doc["status"] = "success";
            doc["message"] = "Ping successful";
        } else {
            doc["status"] = "error";
            doc["message"] = "Ping failed";
        }
        String jsonString;
        serializeJson(doc, jsonString);
        Serial.println(jsonString);
    } else {
        if (Ping.ping(ip.c_str(), 1)) {
            Serial.printf("Ping successful: %s\n", host.c_str());
        } else {
            Serial.printf("Ping failed: %s\n", host.c_str());
        }
    }
}

int countActiveHosts() {
    int active = 0;
    for (int i = 0; i < hostCount; i++) {
        if (hosts[i].isActive) active++;
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