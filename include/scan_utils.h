#ifndef SCAN_UTILS_H
#define SCAN_UTILS_H

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266Ping.h>
#include "structs.h"
#include "wifi_utils.h"
#include "config.h"
#include "localization.h"

void initHostScan(bool scanAll, String startIP, String endIP);
String resolveHost(String host);
void scanHosts();
void scanPorts();
void updateProgressBar();
int countActiveHosts();
void clearMemory();
bool tcpPing(String host, int port);
void manualPing(String host);
String resolveHost(String host);

#endif