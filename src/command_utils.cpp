#include <ESP8266WiFi.h>
#include <ESP8266Ping.h>
#include <ArduinoJson.h>
#include "command_utils.h"
#include "scan_utils.h"
#include "http_utils.h"
#include "structs.h"
#include "wifi_utils.h"
#include "config.h"

void clearWiFiCredentials() {
    clearSavedWiFi();
}

void parseCommand(String cmd) {
    if (browserMode) {
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, cmd);
        if (error) {
            Serial.println("{\"status\":\"error\",\"message\":\"Invalid JSON\"}");
            return;
        }
        String command = doc["command"].as<String>();
        if (command == "set_mode" && doc.containsKey("mode")) {
            String mode = doc["mode"].as<String>();
            if (mode == "browser") {
                browserMode = true;
                Serial.println("{\"status\":\"success\",\"message\":\"Browser mode enabled\"}");
                return;
            }
        } else if (command == "scan") {
            String hostsArg = doc["hosts"].as<String>();
            silentMode = doc["silent"] | false;
            debugMode = doc["debug"] | false;
            lastHostsArg = hostsArg;
            if (doc["ports"].is<String>()) {
                String portsArg = doc["ports"].as<String>();
                if (portsArg == "all") {
                    scanMode = "list";
                    portListCount = min(popularPortsCount, 20);
                    for (int i = 0; i < portListCount; i++) {
                        portList[i] = popularPorts[i];
                    }
                } else if (portsArg.indexOf('-') != -1) {
                    int dash = portsArg.indexOf('-');
                    startPort = portsArg.substring(0, dash).toInt();
                    endPort = portsArg.substring(dash + 1).toInt();
                    if (startPort < 1 || endPort > 65535 || startPort > endPort) {
                        Serial.println("{\"status\":\"error\",\"message\":\"Invalid port range\"}");
                        return;
                    }
                    if (endPort - startPort + 1 > 20) {
                        Serial.println("{\"status\":\"error\",\"message\":\"Port range exceeds maximum of 20 ports\"}");
                        return;
                    }
                    scanMode = "range";
                } else if (portsArg.indexOf(',') != -1) {
                    portListCount = 0;
                    int start = 0;
                    int comma = portsArg.indexOf(',');
                    while (comma != -1 && portListCount < 20) {
                        int port = portsArg.substring(start, comma).toInt();
                        if (port >= 1 && port <= 65535) {
                            portList[portListCount++] = port;
                        }
                        start = comma + 1;
                        comma = portsArg.indexOf(',', start);
                    }
                    int port = portsArg.substring(start).toInt();
                    if (port >= 1 && port <= 65535 && portListCount < 20) {
                        portList[portListCount++] = port;
                    }
                    if (portListCount == 0) {
                        Serial.println("{\"status\":\"error\",\"message\":\"Invalid port list\"}");
                        return;
                    }
                    scanMode = "list";
                } else {
                    int port = portsArg.toInt();
                    if (port < 1 || port > 65535) {
                        Serial.println("{\"status\":\"error\",\"message\":\"Invalid port\"}");
                        return;
                    }
                    scanMode = "single";
                    portList[0] = port;
                    portListCount = 1;
                }
            } else if (doc["ports"].is<JsonArray>()) {
                JsonArray portsArray = doc["ports"].as<JsonArray>();
                portListCount = 0;
                if (portsArray.size() > 20) {
                    Serial.println("{\"status\":\"error\",\"message\":\"Too many ports, maximum is 20\"}");
                    return;
                }
                for (JsonVariant port : portsArray) {
                    int p = port.as<int>();
                    if (p >= 1 && p <= 65535) {
                        portList[portListCount++] = (uint16_t)p;
                    }
                }
                if (portListCount == 0) {
                    Serial.println("{\"status\":\"error\",\"message\":\"No valid ports provided\"}");
                    return;
                }
                scanMode = "list";
            } else {
                Serial.println("{\"status\":\"error\",\"message\":\"Invalid ports format\"}");
                return;
            }
            bool isLocal = (hostsArg == "all" || hostsArg.indexOf('-') != -1);
            initHostScan(isLocal, hostsArg, "");
            totalSteps = hostCount;
            currentStep = 0;
            progress = 0.0;
            Serial.println("{\"status\":\"success\",\"message\":\"Starting scan\"}");
            scanningHosts = true;
        } else if (command == "stop") {
            stopScan();
            return;
        } else if (command == "ping") {
            String host = doc["host"].as<String>();
            manualPing(host);
            return;
        } else if (command == "clear_wifi") {
            clearWiFiCredentials();
            return;
        } else if (command == "lang" && doc.containsKey("language")) {
            String lang = doc["language"].as<String>();
            if (lang.isEmpty()) {
                Serial.println("{\"status\":\"error\",\"message\":\"Invalid language code\"}");
                return;
            }
            Serial.println("{\"status\":\"success\",\"message\":\"Language set to " + lang + "\"}");
        } else {
            Serial.println("{\"status\":\"error\",\"message\":\"Unknown command\"}");
        }
    } else {
        if (cmd == "help") {
            printHelp();
            return;
        }
        if (cmd.startsWith("set_mode browser")) {
            browserMode = true;
            Serial.println("Browser mode enabled");
            return;
        }
        if (cmd == "clear_wifi") {
            clearWiFiCredentials();
            return;
        }
        if (cmd.startsWith("lang ")) {
            String lang = cmd.substring(5);
            if (lang.isEmpty()) {
                Serial.println("Invalid language command");
                return;
            }
            Serial.println("Language set to " + lang);
        } else if (cmd.startsWith("scan ")) {
            int firstSpace = cmd.indexOf(' ');
            int secondSpace = cmd.indexOf(' ', firstSpace + 1);
            int thirdSpace = cmd.indexOf(' ', secondSpace + 1);
            int fourthSpace = cmd.indexOf(' ', thirdSpace + 1);
            if (firstSpace == -1 || secondSpace == -1) {
                Serial.println("Invalid command: scan <hosts> <ports> [--silent|--debug]");
                return;
            }
            lastHostsArg = cmd.substring(firstSpace + 1, secondSpace);
            String portsArg = cmd.substring(secondSpace + 1, thirdSpace == -1 ? cmd.length() : thirdSpace);
            silentMode = false;
            debugMode = false;
            if (thirdSpace != -1) {
                String arg1 = cmd.substring(thirdSpace + 1, fourthSpace == -1 ? cmd.length() : fourthSpace);
                if (arg1 == "--silent") silentMode = true;
                else if (arg1 == "--debug") debugMode = true;
                if (fourthSpace != -1) {
                    String arg2 = cmd.substring(fourthSpace + 1);
                    if (arg2 == "--silent") silentMode = true;
                    else if (arg2 == "--debug") debugMode = true;
                }
            }
            if (debugMode) {
                Serial.println("[DEBUG] Command: " + cmd);
                Serial.println("[DEBUG] Hosts: " + lastHostsArg + ", Ports: " + portsArg + ", Silent mode: " + String(silentMode) + ", Debug mode: " + String(debugMode));
            }
            bool isLocal = (lastHostsArg == "all" || lastHostsArg.indexOf('-') != -1);
            initHostScan(isLocal, lastHostsArg, portsArg);
            if (portsArg == "all") {
                scanMode = "list";
                portListCount = min(popularPortsCount, 20);
                for (int i = 0; i < portListCount; i++) {
                    portList[i] = popularPorts[i];
                }
            } else if (portsArg.indexOf('-') != -1) {
                int dash = portsArg.indexOf('-');
                startPort = portsArg.substring(0, dash).toInt();
                endPort = portsArg.substring(dash + 1).toInt();
                if (startPort < 1 || endPort > 65535 || startPort > endPort) {
                    Serial.println("Invalid port range");
                    if (debugMode) Serial.println("[DEBUG] Invalid port range: start=" + String(startPort) + ", end=" + String(endPort));
                    return;
                }
                if (endPort - startPort + 1 > 20) {
                    Serial.println("Port range exceeds maximum of 20 ports");
                    return;
                }
                scanMode = "range";
            } else if (portsArg.indexOf(',') != -1) {
                portListCount = 0;
                int start = 0;
                int comma = portsArg.indexOf(',');
                while (comma != -1 && portListCount < 20) {
                    int port = portsArg.substring(start, comma).toInt();
                    if (port >= 1 && port <= 65535) {
                        portList[portListCount++] = port;
                    }
                    start = comma + 1;
                    comma = portsArg.indexOf(',', start);
                }
                int port = portsArg.substring(start).toInt();
                if (port >= 1 && port <= 65535 && portListCount < 20) {
                    portList[portListCount++] = port;
                }
                if (portListCount == 0) {
                    Serial.println("Invalid port list");
                    if (debugMode) Serial.println("[DEBUG] Invalid port list: " + portsArg);
                    return;
                }
                scanMode = "list";
            } else {
                int port = portsArg.toInt();
                if (port < 1 || port > 65535) {
                    Serial.println("Invalid port");
                    if (debugMode) Serial.println("[DEBUG] Invalid port: " + portsArg);
                    return;
                }
                scanMode = "single";
                portList[0] = port;
                portListCount = 1;
            }
            totalSteps = hostCount;
            currentStep = 0;
            progress = 0.0;
            if (debugMode) {
                Serial.println("[DEBUG] Scan mode: " + scanMode + ", Hosts: " + String(hostCount) + ", Total steps: " + String(totalSteps));
                Serial.println("[DEBUG] Ports: " + (scanMode == "range" ? (String(startPort) + "-" + String(endPort)) : String(portListCount) + " ports"));
            }
            Serial.println("Starting scan...");
            scanningHosts = true;
        } else if (cmd.startsWith("ping")) {
            int space = cmd.indexOf(' ');
            if (space == -1) {
                Serial.println("Invalid command: ping <IP|URL>");
                return;
            }
            String host = cmd.substring(space + 1);
            manualPing(host);
        } else if (cmd == "stop") {
            stopScan();
        } else {
            Serial.println("Unknown command. Enter 'help'");
        }
    }
}

void printHelp() {
    if (browserMode) {
        StaticJsonDocument<1024> doc;
        doc["status"] = "success";
        doc["message"] = "Commands: scan, ping, stop, set_mode, clear_wifi";
        String jsonString;
        serializeJson(doc, jsonString);
        Serial.println(jsonString);
    } else {
        Serial.println("=== Scanner Help ===");
        Serial.println("Commands:");
        Serial.println("  scan <hosts> <ports> [--silent|--debug]");
        Serial.println("    - Scans hosts and ports");
        Serial.println("    - Hosts: all, <IP>, <URL>, <IP_start>-<IP_end>, <IP1>,<IP2>,<URL>");
        Serial.println("    - Ports: all, <port>, <port_start>-<port_end>, <port1>,<port2>");
        Serial.println("    - Options: --silent, --debug");
        Serial.println("  ping <IP|URL> - Pings IP/URL");
        Serial.println("  stop - Stops scan");
        Serial.println("  set_mode browser - Enables browser mode");
        Serial.println("  clear_wifi - Clears saved Wi-Fi credentials");
        Serial.println("  help - Shows help");
        Serial.println("=====================");
    }
}

void stopScan() {
    scanningHosts = false;
    scanningPorts = false;
    clearMemory();
    if (browserMode) {
        Serial.println("{\"status\":\"success\",\"message\":\"Scan stopped\"}");
    } else {
        Serial.println("Scan stopped");
    }
}

void processLocationName(String input) {
    input.trim();
    if (input.length() > 0) {
        locationName = input;
    }
    uploadScanReport();
    uploadReport = false;
    awaitingLocationName = false;
    clearMemory();
}

void processUploadDecision(String input) {
    input.trim();
    if (browserMode) {
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(doc, input);
        if (error || !doc.containsKey("decision")) {
            Serial.println("{\"status\":\"error\",\"message\":\"Invalid JSON or missing decision\"}");
            return;
        }
        input = doc["decision"].as<String>();
    }
    if (input == "yes") {
        uploadReport = true;
        if (browserMode) {
            Serial.println("{\"status\":\"prompt\",\"message\":\"Enter location name (or press Enter for default)\"}");
        } else {
            Serial.println("Enter location name (or press Enter for default):");
        }
        awaitingLocationName = true;
    } else if (input == "no") {
        uploadReport = false;
        if (browserMode) {
            Serial.println("{\"status\":\"success\",\"message\":\"Upload skipped\"}");
        } else {
            Serial.println("Upload skipped");
        }
        clearMemory();
    } else {
        if (browserMode) {
            Serial.println("{\"status\":\"error\",\"message\":\"Invalid upload decision, expected 'yes' or 'no'\"}");
        } else {
            Serial.println("Invalid upload input");
        }
        return;
    }
}