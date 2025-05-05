#ifndef STRUCTS_H
#define STRUCTS_H

#include <Arduino.h>

// Платформо-зависимые настройки
#ifdef ESP32
    #define MAX_OPEN_PORTS 100
    #define MAX_HOSTS 256
    #define MAX_PORT_LIST 100
    typedef uint16_t port_type_t;
#else // ESP8266
    #define MAX_OPEN_PORTS 20
    #define MAX_HOSTS 30
    #define MAX_PORT_LIST 20
    typedef int port_type_t;
#endif

struct Host {
    String ip;
    bool isActive;
    port_type_t openPorts[MAX_OPEN_PORTS];
    String services[20];
    int openPortCount;
};

// Общие переменные
extern String selectedSSID;
extern String selectedPassword;
extern bool awaitingPassword;
extern bool awaitingNetworkSelection;
extern int networkCount;
extern String lastHostsArg;
extern String scanMode;
extern int startPort;
extern int endPort;
extern port_type_t portList[MAX_PORT_LIST];
extern int portListCount;
extern int hostCount;
extern Host hosts[MAX_HOSTS];
extern bool scanningHosts;
extern bool scanningPorts;
extern int totalSteps;
extern int currentStep;
extern float progress;
extern bool debugMode;
extern bool silentMode;
extern bool uploadReport;
extern int currentHostIndex;
extern int currentPortIndex;
extern int currentPort;
extern bool deviceConnected;
extern String apiKey;
extern bool browserMode;
extern bool awaitingLocationName;

#endif