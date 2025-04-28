#include "command_utils.h"
#include "scan_utils.h"
#include "wifi_utils.h"
#include "http_utils.h"
#include "structs.h"

void parseCommand(String cmd) {
  if (debugMode) {
    Serial.println("[DEBUG] Command received: " + cmd);
    Serial.flush();
  }

  if (cmd == "help") {
    printHelp();
    return;
  }

  if (cmd.startsWith("lang ")) {
    String lang = cmd.substring(5);
    if (lang.isEmpty()) {
      Serial.println("Invalid language command");
      return;
    }
  } else if (cmd.startsWith("scan ")) {
    int firstSpace = cmd.indexOf(' ');
    int secondSpace = cmd.indexOf(' ', firstSpace + 1);
    int thirdSpace = cmd.indexOf(' ', secondSpace + 1);
    int fourthSpace = cmd.indexOf(' ', thirdSpace + 1);

    if (firstSpace == -1 || secondSpace == -1) {
      Serial.println("Invalid command: scan <hosts> <ports> [--silent|--debug]");
      Serial.flush();
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
      Serial.flush();
    }

    bool isLocal = (lastHostsArg == "all" || lastHostsArg.indexOf('-') != -1);
    initHostScan(isLocal, lastHostsArg, portsArg); // Упрощён вызов

    if (portsArg == "all") {
      scanMode = "list";
      portListCount = min(popularPortsCount, 20); // Ограничение до 20
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
        Serial.flush();
        return;
      }
      scanMode = "range";
    } else if (portsArg.indexOf(',') != -1) {
      portListCount = 0;
      int start = 0;
      int comma = portsArg.indexOf(',');
      while (comma != -1 && portListCount < 20) { // Ограничение до 20
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
        Serial.flush();
        return;
      }
      scanMode = "list";
    } else {
      int port = portsArg.toInt();
      if (port < 1 || port > 65535) {
        Serial.println("Invalid port");
        if (debugMode) Serial.println("[DEBUG] Invalid port: " + portsArg);
        Serial.flush();
        return;
      }
      scanMode = "single";
      portList[0] = port;
      portListCount = 1;
    }

    totalSteps = hostCount;
    if (scanMode == "range") {
      totalSteps += countActiveHosts() * (endPort - startPort + 1);
    } else {
      totalSteps += countActiveHosts() * portListCount;
    }
    currentStep = 0;
    progress = 0.0;

    if (debugMode) {
      Serial.println("[DEBUG] Scan mode: " + scanMode + ", Hosts: " + String(hostCount) + ", Total steps: " + String(totalSteps));
      Serial.println("[DEBUG] Ports: " + (scanMode == "range" ? (String(startPort) + "-" + String(endPort)) : String(portListCount) + " ports"));
      Serial.flush();
    }

    String modeInfo = silentMode ? " (silent mode)" : (debugMode ? " (debug mode)" : "");
    Serial.printf("Starting scan...%s\n", modeInfo.c_str());
    Serial.flush();
    scanningHosts = true;
  } else if (cmd.startsWith("ping")) {
    int space = cmd.indexOf(' ');
    if (space == -1) {
      Serial.println("Invalid command: ping <IP|URL>");
      Serial.flush();
      return;
    }
    String host = cmd.substring(space + 1);
    manualPing(host);
  } else if (cmd == "stop") {
    stopScan(); // Вызываем отдельную функцию
  } else {
    Serial.println("Unknown command. Enter 'help'");
    Serial.flush();
  }
}

void printHelp() {
  Serial.println("=== Scanner Help ===");
  Serial.println("Commands:");
  Serial.println("  scan <hosts> <ports> [--silent|--debug]");
  Serial.println("    - Scans hosts and ports");
  Serial.println("    - Hosts:");
  Serial.println("      - all: Local subnet");
  Serial.println("      - <IP>: Single IP");
  Serial.println("      - <URL>: Domain");
  Serial.println("      - <IP_start>-<IP_end>: IP range");
  Serial.println("      - <IP1>,<IP2>,<URL>: List");
  Serial.println("    - Ports:");
  Serial.println("      - all: Popular ports");
  Serial.println("      - <port>: Single port");
  Serial.println("      - <port_start>-<port_end>: Port range");
  Serial.println("      - <port1>,<port2>: Port list");
  Serial.println("    - Options:");
  Serial.println("      - --silent: Progress only");
  Serial.println("      - --debug: Detailed debug");
  Serial.println("    Examples:");
  Serial.println("      scan all all --silent");
  Serial.println("      scan 8.8.8.8 53 --debug");
  Serial.println("      scan google.com 80,443");
  Serial.println("      scan 192.168.0.20-192.168.0.40 5000");
  Serial.println("      scan 192.168.0.20,google.com 80-100 --silent");
  Serial.println("  ping <IP|URL>");
  Serial.println("    - Pings IP/URL");
  Serial.println("  stop");
  Serial.println("    - Stops scan");
  Serial.println("  help");
  Serial.println("    - Shows help");
  Serial.println("  lang <language_code>");
  Serial.println("    - Changes language (en)");
  Serial.println("After scan: 'yes'/'no' to upload to netscout.tech");
  Serial.println("Local scans: IP from ipify.org");
  Serial.println("Remote scans: First IP/URL");
  Serial.println("=====================");
  Serial.flush();
}

void stopScan() {
  scanningHosts = false;
  scanningPorts = false;
  clearMemory();
  Serial.println("Scan stopped");
  Serial.flush();
}

void processUploadDecision(String input) {
  input.trim();
  if (input == "yes") {
    uploadReport = true; // Включаем загрузку
    uploadScanReport();
  } else if (input == "no") {
    uploadReport = false;
    Serial.println("Upload skipped");
    Serial.flush();
  } else {
    Serial.println("Invalid upload input");
    Serial.flush();
    return;
  }
  uploadReport = false;
  clearMemory();
}