#ifndef COMMAND_UTILS_H
#define COMMAND_UTILS_H

#include <Arduino.h>

void parseCommand(String cmd);
void printHelp();
void stopScan();
void processUploadDecision(String input);
void clearWiFiCredentials();
void processLocationName(String input);

#endif