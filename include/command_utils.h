#ifndef COMMAND_UTILS_H
#define COMMAND_UTILS_H

#include <WString.h>
#include "structs.h"
#include "scan_utils.h"
#include "http_utils.h"

void parseCommand(String cmd);
void printHelp();
void stopScan(); // Добавляем объявление
void processUploadDecision(String input);

#endif