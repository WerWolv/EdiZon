#pragma once
#include <edizon.h>

#define DEBUG_LOG(statement, formatStr) (printf("%s : %d %s -> " formatStr "\n", __FUNCTION__, __LINE__, #statement, statement))

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