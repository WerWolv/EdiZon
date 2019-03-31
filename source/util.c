#include "util.h"

#include <stdio.h>
#include <time.h>

static Service g_pmdmntService;
static u64 g_refCnt;

Result amspmdmntInitialize(void) {
  atomicIncrement64(&g_refCnt);

  if (g_pmdmntService.type != ServiceType_Uninitialized)
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
  bool running = R_FAILED(smRegisterService(&handle, serviceName, false, 1));

  svcCloseHandle(handle);

  if (!running)
    smUnregisterService(serviceName);

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