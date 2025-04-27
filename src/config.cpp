#include "config.h"

const int popularPorts[] = {20, 21, 22, 23, 25, 53, 67, 68, 69, 80, 110, 123, 135, 137, 138, 139, 143, 161, 162, 179, 389, 443, 445, 465, 514, 587, 993, 995, 1433, 1521, 1723, 3306, 3389, 5000, 5060, 5061, 5432, 5900, 6379, 8080, 8443, 9000};
const int popularPortsCount = sizeof(popularPorts) / sizeof(popularPorts[0]);
String serverUrl = "http://192.168.10.104:5000/upload";