#ifndef WIFI_UTILS_H
#define WIFI_UTILS_H

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <FS.h>
#include "structs.h"

void scanWiFiNetworks();
void processWiFiSelection(String input);
void connectToWiFi();
bool isLocalIP(String ip);
bool isIPAddress(String str);
bool hasInternet(); // Проверка интернета
void promptDeviceConnection(); // Предложение подключить устройство
void processDeviceCode(String input); // Обработка кода от пользователя

#endif