#pragma once
#include <switch.h>

#define DEBUG_LOG(statement, formatStr) (printf("%s : %d %s -> " formatStr "\n", __FUNCTION__, __LINE__, #statement, statement))

#define API_VERSION "v2"
#define EDIZON_URL "https://vps.werwolv.net/api/edizon/" API_VERSION 

#ifdef __cplusplus
extern "C" {
#endif

Result amspmdmntInitialize(void);
void amspmdmntExit(void);

Result amspmdmntAtmosphereGetProcessHandle(Handle *out_processHandle);

bool isServiceRunning(const char *serviceName);
void getCurrTimeString(char *buffer);
void getCurrBatteryPercentage(char *buffer);

void ledInit();
void setLedState(bool state);

void overclockSystem(bool enable);

#ifdef __cplusplus
}
#endif