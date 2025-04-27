#ifndef STRUCTS_H
#define STRUCTS_H

#include <WString.h>

struct Host {
  String ip;
  bool isActive;
  int openPorts[20];
  String services[20];
  int openPortCount;
};

extern String selectedSSID;
extern String selectedPassword;
extern bool awaitingPassword;
extern bool awaitingNetworkSelection;
extern int networkCount;
extern String lastHostsArg;
extern String scanMode;
extern int startPort;
extern int endPort;
extern int portList[43];
extern int portListCount;
extern int hostCount;
extern Host hosts[30];
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
extern bool deviceConnected; // Для статуса подключения к netscout.tech
extern String apiKey; // Храним API-ключ

#endif