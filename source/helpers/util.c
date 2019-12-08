#include "helpers/util.h"

#include <stdio.h>
#include <time.h>
#include <string.h>

#define MHz *1E6

static Service g_pmdmntService;
static u64 g_refCnt;

static u64 g_uniquePadIds[2];
static size_t g_uniquePadCnt;
static HidsysNotificationLedPattern g_patternOn, g_patternOff;

Result amspmdmntInitialize(void) {
  atomicIncrement64(&g_refCnt);

  if (serviceIsActive(&g_pmdmntService))
    return 1;

  return smGetService(&g_pmdmntService, "pm:dmnt");
}

void amspmdmntExit(void) {
  if (atomicDecrement64(&g_refCnt) == 0) {
    serviceClose(&g_pmdmntService);
  }
}

bool isServiceRunning(const char *serviceName) {
  Handle handle;
  SmServiceName service_name = smEncodeName(serviceName);
  bool running = R_FAILED(smRegisterService(&handle, service_name, false, 1));

  svcCloseHandle(handle);

  if (!running)
    smUnregisterService(service_name);

  return running;
}

void getCurrTimeString(char *buffer) {
  time_t unixTime = time(NULL);
  struct tm* time = localtime((const time_t*)&unixTime);
  sprintf(buffer, "%02d:%02d", time->tm_hour, time->tm_min);
}

void getCurrBatteryPercentage(char *buffer) {
  u32 percents = 0;
  psmGetBatteryChargePercentage(&percents);
  sprintf(buffer, "%d%%", percents);
}

void ledInit() {
  hidsysGetUniquePadsFromNpad(hidGetHandheldMode() ? CONTROLLER_HANDHELD : CONTROLLER_PLAYER_1, g_uniquePadIds, 2, &g_uniquePadCnt);

  memset(&g_patternOn, 0x00, sizeof(HidsysNotificationLedPattern));
  memset(&g_patternOff, 0x00, sizeof(HidsysNotificationLedPattern));

  g_patternOn.baseMiniCycleDuration = 0x0F;
  g_patternOn.startIntensity = 0x0F;
  g_patternOn.miniCycles[0].ledIntensity = 0x0F;
  g_patternOn.miniCycles[0].transitionSteps = 0x0F;
  g_patternOn.miniCycles[0].finalStepDuration = 0x0F;
}

void setLedState(bool state) {
  for(u8 i = 0; i < g_uniquePadCnt; i++)
    hidsysSetNotificationLedPattern(state ? &g_patternOn : &g_patternOff, g_uniquePadIds[i]);
}

void overclockSystem(bool enable) {
  if (hosversionBefore(8, 0, 0)) {
    pcvSetClockRate(PcvModule_CpuBus, enable ? 1785 MHz : 1020 MHz);  // Set CPU clock
    pcvSetClockRate(PcvModule_EMC, enable ? 1600 MHz : 1331 MHz);     // Set memory clock
  } else {
    ClkrstSession clkrstSession;
    clkrstOpenSession(&clkrstSession, PcvModuleId_CpuBus, 3);
    clkrstSetClockRate(&clkrstSession, enable ? 1785 MHz : 1020 MHz); // Set CPU clock
    clkrstCloseSession(&clkrstSession);

    clkrstOpenSession(&clkrstSession, PcvModuleId_EMC, 3);
    clkrstSetClockRate(&clkrstSession, enable ? 1600 MHz : 1331 MHz); // Set memory clock
    clkrstCloseSession(&clkrstSession);
  }
}

Result amspmdmntAtmosphereGetProcessHandle(Handle *out_processHandle) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65000;

  Result rc = serviceIpcDispatch(&g_pmdmntService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_pmdmntService, &r, sizeof(*resp));
    resp = r.Raw;

    *out_processHandle = r.Handles[0];

    rc = resp->result;
  }

  return rc;
}