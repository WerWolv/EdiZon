#pragma once
#include <switch.h>

Result amspmdmntInitialize(void);
void amspmdmntExit(void);

Result amspmdmntAtmosphereGetProcessHandle(Handle *out_processHandle);

bool isServiceRunning(const char *serviceName);
void getCurrTimeString(char *buffer);
void getCurrBatteryPercentage(char *buffer);