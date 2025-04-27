#include "localization.h"

// Определяем langJson только с английским языком
const char Localization::langJson[] PROGMEM = R"LANG({
  "en": {
    "SCANNING_WIFI": "Scanning Wi-Fi networks...",
    "NETWORKS_NOT_FOUND": "No networks found. Retrying in 5s...",
    "FOUND_NETWORKS": "Found %d networks:",
    "NETWORK_ITEM": "%d: %s (Signal: %d dBm)",
    "ENTER_WIFI_NUMBER": "Enter Wi-Fi network number (1-%d):",
    "INVALID_NUMBER": "Invalid number. Enter 1 to %d",
    "SELECTED_SSID": "Selected: %s",
    "ENTER_PASSWORD": "Enter password for %s:",
    "CONNECTING_TO": "Connecting to %s...",
    "CONNECTED": "Connected! IP: %s",
    "CONNECTION_FAILED": "Connection failed. Try again.",
    "HELP_PROMPT": "Enter 'help' for commands.",
    "INVALID_IP_RANGE": "Invalid IP range.",
    "NO_VALID_HOSTS": "No valid hosts to scan.",
    "HOST_SCAN_COMPLETE": "Host scan done. Found %d active.",
    "UPLOAD_PROMPT": "Upload report to server? (yes/no)",
    "PORT_SCAN_COMPLETE": "Port scan completed.",
    "PINGING": "Pinging %s...",
    "HOST_ACTIVE": "ACTIVE",
    "HOST_INACTIVE": "INACTIVE",
    "SCANNING_PORT": "Scanning %s:%d...",
    "PORT_OPEN": "OPEN",
    "PORT_CLOSED": "CLOSED",
    "INVALID_COMMAND": "Invalid command: %s",
    "INVALID_PORT_RANGE": "Invalid port range.",
    "INVALID_PORT_LIST": "Invalid port list.",
    "INVALID_PORT": "Invalid port.",
    "SCAN_START": "Starting scan...%s",
    "SCAN_STOPPED": "Scan stopped.",
    "UNKNOWN_COMMAND": "Unknown command. Enter 'help'.",
    "REPORT_UPLOADED": "Report uploaded: %s",
    "REPORT_UPLOAD_FAILED": "Upload failed. Code: %d",
    "UPLOAD_SKIPPED": "Upload skipped.",
    "INVALID_UPLOAD_INPUT": "Enter 'yes' or 'no'.",
    "DNS_FAILED": "DNS failed for %s",
    "PORT_SCAN_FINISHED": "Port scan for %s finished",
    "PING_FAILED": "Failed to ping: %s",
    "EXTERNAL_IP_FAILED": "Failed to get external IP",
    "INVALID_LANG_COMMAND": "Invalid language command.",
    "LANG_SET": "Language set: %s",
    "LANG_UNSUPPORTED": "Unsupported language: %s",
    "HELP_HEADER": "=== Scanner Help ===",
    "HELP_COMMANDS": "Commands:",
    "HELP_SCAN": "  scan <hosts> <ports> [--silent|--debug]",
    "HELP_SCAN_DESC": "    - Scans hosts and ports.",
    "HELP_HOSTS": "    - Hosts:",
    "HELP_ALL": "      - all: Local subnet",
    "HELP_SINGLE_IP": "      - <IP>: Single IP",
    "HELP_URL": "      - <URL>: Domain",
    "HELP_IP_RANGE": "      - <IP_start>-<IP_end>: IP range",
    "HELP_IP_LIST": "      - <IP1>,<IP2>,<URL>: List",
    "HELP_PORTS": "    - Ports:",
    "HELP_ALL_PORTS": "      - all: Popular ports",
    "HELP_SINGLE_PORT": "      - <port>: Single port",
    "HELP_PORT_RANGE": "      - <port_start>-<port_end>: Port range",
    "HELP_PORT_LIST": "      - <port1>,<port2>: Port list",
    "HELP_OPTIONS": "    - Options:",
    "HELP_SILENT": "      - --silent: Progress only.",
    "HELP_DEBUG": "      - --debug: Detailed debug.",
    "HELP_EXAMPLES": "    Examples:",
    "HELP_EXAMPLE_1": "      scan all all --silent",
    "HELP_EXAMPLE_2": "      scan 8.8.8.8 53 --debug",
    "HELP_EXAMPLE_3": "      scan google.com 80,443",
    "HELP_EXAMPLE_4": "      scan 192.168.0.20-192.168.0.40 5000",
    "HELP_EXAMPLE_5": "      scan 192.168.0.20,google.com 80-100 --silent",
    "HELP_PING": "  ping <IP|URL>",
    "HELP_PING_DESC": "    - Pings IP/URL",
    "HELP_STOP": "  stop",
    "HELP_STOP_DESC": "    - Stops scan.",
    "HELP_HELP": "  help",
    "HELP_HELP_DESC": "    - Shows help.",
    "HELP_LANG": "  lang <language_code>",
    "HELP_LANG_DESC": "    - Changes language (en).",
    "HELP_UPLOAD_INFO": "After scan: 'yes'/'no' to upload to %s.",
    "HELP_LOCATION_LOCAL": "Local scans: IP from ipify.org.",
    "HELP_LOCATION_REMOTE": "Remote scans: First IP/URL.",
    "HELP_FOOTER": "====================="
  }
})LANG";

Localization loc; // Определение глобального объекта

void Localization::init() {
  String jsonString;
  jsonString.reserve(strlen_P(langJson));
  for (size_t i = 0; i < strlen_P(langJson); i++) {
    jsonString += (char)pgm_read_byte(&langJson[i]);
  }
  
  DeserializationError error = deserializeJson(langDoc, jsonString);
  if (error) {
    Serial.print(F("JSON parse failed: "));
    Serial.println(error.c_str());
  }
}

void Localization::setLanguage(String lang) {
  if (langDoc.containsKey(lang)) {
    currentLang = lang;
    Serial.println(getString("LANG_SET", lang));
  } else {
    Serial.println(getString("LANG_UNSUPPORTED", lang));
  }
}

String Localization::getString(String key) {
  return langDoc[currentLang][key] | "MISSING_STRING";
}

String Localization::getString(String key, String arg1) {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), langDoc[currentLang][key] | "MISSING_STRING", arg1.c_str());
  return String(buffer);
}

String Localization::getString(String key, int arg1) {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), langDoc[currentLang][key] | "MISSING_STRING", arg1);
  return String(buffer);
}

String Localization::getString(String key, String arg1, String arg2) {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), langDoc[currentLang][key] | "MISSING_STRING", arg1.c_str(), arg2.c_str());
  return String(buffer);
}

String Localization::getString(String key, String arg1, int arg2) {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), langDoc[currentLang][key] | "MISSING_STRING", arg1.c_str(), arg2);
  return String(buffer);
}

String Localization::getString(String key, int arg1, int arg2) {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), langDoc[currentLang][key] | "MISSING_STRING", arg1, arg2);
  return String(buffer);
}

String Localization::getString(String key, int arg1, String arg2, int arg3) {
  char buffer[256];
  snprintf(buffer, sizeof(buffer), langDoc[currentLang][key] | "MISSING_STRING", arg1, arg2.c_str(), arg3);
  return String(buffer);
}