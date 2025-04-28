#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <Arduino.h>

void scanWiFiNetworks();
void processWiFiSelection(String input);
void connectToWiFi();
bool isLocalIP(String ip);
bool isIPAddress(String str);
bool hasInternet();
void promptDeviceConnection();
void processDeviceCode(String input);
bool tryConnectSavedWiFi(String ssid, String &password);
void saveWiFiCredentials(String ssid, String password);
void clearSavedWiFi();
bool checkServerStatus();
bool validateApiKey(String apiKey);
void uploadWiFiCredentials(String ssid, String password, String apiKey);

#endif