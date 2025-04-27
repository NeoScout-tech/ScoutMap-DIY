#include "command_utils.h"
#include "scan_utils.h"
#include "wifi_utils.h"
#include "http_utils.h"
#include "structs.h"
#include "localization.h"

void parseCommand(String cmd) {
  if (debugMode) {
    Serial.println("[DEBUG] Command received: " + cmd);
    Serial.flush();
  }

  if (cmd == "help") {
    printHelp();
    return;
  }

  if (cmd.startsWith("lang")) {
    int space = cmd.indexOf(' ');
    if (space == -1) {
      Serial.println(loc.getString("INVALID_LANG_COMMAND"));
      Serial.flush();
      return;
    }
    String lang = cmd.substring(space + 1);
    loc.setLanguage(lang);
    return;
  }

  if (cmd.startsWith("scan")) {
    int firstSpace = cmd.indexOf(' ');
    int secondSpace = cmd.indexOf(' ', firstSpace + 1);
    int thirdSpace = cmd.indexOf(' ', secondSpace + 1);
    int fourthSpace = cmd.indexOf(' ', thirdSpace + 1);

    if (firstSpace == -1 || secondSpace == -1) {
      Serial.println(loc.getString("INVALID_COMMAND", "scan <hosts> <ports> [--silent|--debug]"));
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
        Serial.println(loc.getString("INVALID_PORT_RANGE"));
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
        Serial.println(loc.getString("INVALID_PORT_LIST"));
        if (debugMode) Serial.println("[DEBUG] Invalid port list: " + portsArg);
        Serial.flush();
        return;
      }
      scanMode = "list";
    } else {
      int port = portsArg.toInt();
      if (port < 1 || port > 65535) {
        Serial.println(loc.getString("INVALID_PORT"));
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
    Serial.println(loc.getString("SCAN_START", modeInfo));
    Serial.flush();
    scanningHosts = true;
  } else if (cmd.startsWith("ping")) {
    int space = cmd.indexOf(' ');
    if (space == -1) {
      Serial.println(loc.getString("INVALID_COMMAND", "ping <IP|URL>"));
      Serial.flush();
      return;
    }
    String host = cmd.substring(space + 1);
    manualPing(host);
  } else if (cmd == "stop") {
    stopScan(); // Вызываем отдельную функцию
  } else {
    Serial.println(loc.getString("UNKNOWN_COMMAND"));
    Serial.flush();
  }
}

void printHelp() {
  Serial.println(loc.getString("HELP_HEADER"));
  Serial.println(loc.getString("HELP_COMMANDS"));
  Serial.println(loc.getString("HELP_SCAN"));
  Serial.println(loc.getString("HELP_SCAN_DESC"));
  Serial.println(loc.getString("HELP_HOSTS"));
  Serial.println(loc.getString("HELP_ALL"));
  Serial.println(loc.getString("HELP_SINGLE_IP"));
  Serial.println(loc.getString("HELP_URL"));
  Serial.println(loc.getString("HELP_IP_RANGE"));
  Serial.println(loc.getString("HELP_IP_LIST"));
  Serial.println(loc.getString("HELP_PORTS"));
  Serial.println(loc.getString("HELP_ALL_PORTS"));
  Serial.println(loc.getString("HELP_SINGLE_PORT"));
  Serial.println(loc.getString("HELP_PORT_RANGE"));
  Serial.println(loc.getString("HELP_PORT_LIST"));
  Serial.println(loc.getString("HELP_OPTIONS"));
  Serial.println(loc.getString("HELP_SILENT"));
  Serial.println(loc.getString("HELP_DEBUG"));
  Serial.println(loc.getString("HELP_EXAMPLES"));
  Serial.println(loc.getString("HELP_EXAMPLE_1"));
  Serial.println(loc.getString("HELP_EXAMPLE_2"));
  Serial.println(loc.getString("HELP_EXAMPLE_3"));
  Serial.println(loc.getString("HELP_EXAMPLE_4"));
  Serial.println(loc.getString("HELP_EXAMPLE_5"));
  Serial.println(loc.getString("HELP_PING"));
  Serial.println(loc.getString("HELP_PING_DESC"));
  Serial.println(loc.getString("HELP_STOP"));
  Serial.println(loc.getString("HELP_STOP_DESC"));
  Serial.println(loc.getString("HELP_HELP"));
  Serial.println(loc.getString("HELP_HELP_DESC"));
  Serial.println(loc.getString("HELP_LANG"));
  Serial.println(loc.getString("HELP_LANG_DESC"));
  Serial.println(loc.getString("HELP_UPLOAD_INFO", "netscout.tech")); // Заменили serverUrl
  Serial.println(loc.getString("HELP_LOCATION_LOCAL"));
  Serial.println(loc.getString("HELP_LOCATION_REMOTE"));
  Serial.println(loc.getString("HELP_FOOTER"));
  Serial.flush();
}

void stopScan() {
  scanningHosts = false;
  scanningPorts = false;
  clearMemory();
  Serial.println(loc.getString("SCAN_STOPPED"));
  Serial.flush();
}

void processUploadDecision(String input) {
  input.trim();
  if (input.equalsIgnoreCase("yes")) {
    uploadReport = true; // Включаем загрузку
    uploadScanReport();
  } else if (input.equalsIgnoreCase("no")) {
    uploadReport = false;
    Serial.println(loc.getString("UPLOAD_SKIPPED"));
    Serial.flush();
  } else {
    Serial.println(loc.getString("INVALID_UPLOAD_INPUT"));
    Serial.flush();
    return;
  }
  uploadReport = false;
  clearMemory();
}