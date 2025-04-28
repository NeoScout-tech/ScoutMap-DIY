#ifndef SCAN_UTILS_H
#define SCAN_UTILS_H

#include <WiFi.h>
#include <ESP32Ping.h>
#include "structs.h"
#include "wifi_utils.h"
#include "config.h"

void initHostScan(bool scanAll, String startIP, String endIP);
String resolveHost(String host);
void scanHosts();
void scanPorts();
int countActiveHosts();
void clearMemory();
bool tcpPing(String host, int port);
void manualPing(String host);

#endif