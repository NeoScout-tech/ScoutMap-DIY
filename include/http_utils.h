#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "structs.h"
#include "config.h" // Добавляем включение config.h
#include "wifi_utils.h"

String getExternalIP();
String getLocation();
void uploadScanReport();

#endif