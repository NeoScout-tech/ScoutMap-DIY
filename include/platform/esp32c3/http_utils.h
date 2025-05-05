#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <WiFi.h>
#include <HTTPClient.h>
#include "structs.h"
#include "config.h"
#include "wifi_utils.h"

String getLocation();
void uploadScanReport();

#endif