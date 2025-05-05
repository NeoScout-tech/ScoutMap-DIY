#ifndef SCAN_UTILS_H
#define SCAN_UTILS_H

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266Ping.h>
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
String resolveHost(String host);
String detectOS(String ip);
String getServiceVersion(String ip, int port, String service);
String scanService(String ip, int port);

#endif