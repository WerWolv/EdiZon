#include "edzn_cht.h"

#include <string.h>

static Service g_edznchtService;
static Service g_pmdmntService;
static u64 g_refCnt;

void edznchtInitialize(void) {
  atomicIncrement64(&g_refCnt);

  if (g_edznchtService.type != ServiceType_Uninitialized)
    return;

  smGetService(&g_edznchtService, "edzn:cht");
  smGetService(&g_pmdmntService, "pm:dmnt");
}

void edznchtExit(void) {
  if (atomicDecrement64(&g_refCnt) == 0) {
    serviceClose(&g_edznchtService);
    serviceClose(&g_pmdmntService);
  }
}

Result edznchtAddMemoryFreeze(u64 freezeAddr, u64 freezeValue, u64 valueType) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 freezeAddr;
    u64 freezeValue;
    u64 valueType;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 0;
  raw->freezeAddr = freezeAddr;
  raw->freezeValue = freezeValue;
  raw->valueType = valueType;

  Result rc = serviceIpcDispatch(&g_edznchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_edznchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}

Result edznchtRemoveMemoryFreeze(u64 freezeAddr) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 freezeAddr;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 1;
  raw->freezeAddr = freezeAddr;

  Result rc = serviceIpcDispatch(&g_edznchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_edznchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}

Result edznchtUpdateMemoryFreeze(u64 freezeAddr, u64 newValue) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 freezeAddr;
    u64 newValue;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 2;
  raw->freezeAddr = freezeAddr;
  raw->newValue = newValue;

  Result rc = serviceIpcDispatch(&g_edznchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_edznchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}

Result edznchtGetFrozenMemoryAddresses(u64 *buffer, size_t bufferSize, size_t *addrsRead) {
  IpcCommand c;
  ipcInitialize(&c);
  ipcAddRecvBuffer(&c, buffer, bufferSize * sizeof(u64), 0);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 3;

  Result rc = serviceIpcDispatch(&g_edznchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
        u32 addrsRead;
    } *resp;

    serviceIpcParse(&g_edznchtService, &r, sizeof(*resp));
    resp = r.Raw;

    *addrsRead = resp->addrsRead;
    rc = resp->result;
  }

  return rc;
}

Result edznchtGetFrozenMemoryAddressCount(size_t *frozenAddrCnt) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 freezeAddr;
    u64 newValue;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 4;

  Result rc = serviceIpcDispatch(&g_edznchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
        size_t frozenAddrCnt;
    } *resp;

    serviceIpcParse(&g_edznchtService, &r, sizeof(*resp));
    resp = r.Raw;

    *frozenAddrCnt = resp->frozenAddrCnt;

    rc = resp->result;
  }

  return rc;
}

Result pmdmntAtmosphereGetProcessHandle (Handle *out_processHandle) {
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

Result edznchtLoadCheat(char *fileName, char *cheatName) {
  IpcCommand c;
  ipcInitialize(&c);

  ipcAddSendBuffer(&c, fileName, strlen(fileName) + 1, 0);
  ipcAddSendBuffer(&c, cheatName, strlen(cheatName) + 1, 0);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 5;

  Result rc = serviceIpcDispatch(&g_edznchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_edznchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}

Result edznchtUnloadCheat(char *fileName, char *cheatName) {
  IpcCommand c;
  ipcInitialize(&c);

  ipcAddSendBuffer(&c, fileName, strlen(fileName) + 1, 0);
  ipcAddSendBuffer(&c, cheatName, strlen(cheatName) + 1, 0);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 6;

  Result rc = serviceIpcDispatch(&g_edznchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_edznchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}

Result edznchtActivateCheat(char *fileName, char *cheatName, bool activated) {
  IpcCommand c;
  ipcInitialize(&c);

  ipcAddSendBuffer(&c, fileName, strlen(fileName) + 1, 0);
  ipcAddSendBuffer(&c, cheatName, strlen(cheatName) + 1, 0);

  struct {
    u64 magic;
    u64 cmd_id;
    bool activated;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 7;
  raw->activated = activated;

  Result rc = serviceIpcDispatch(&g_edznchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_edznchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}

Result edznchtGetLoadedCheats(bool *buffer, size_t bufferSize, size_t *cheatsRead) {
  IpcCommand c;
  ipcInitialize(&c);

  ipcAddRecvBuffer(&c, buffer, bufferSize, 0);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 8;

  Result rc = serviceIpcDispatch(&g_edznchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
        size_t cheatsRead;
    } *resp;

    serviceIpcParse(&g_edznchtService, &r, sizeof(*resp));
    resp = r.Raw;

    *cheatsRead = resp->cheatsRead;

    rc = resp->result;
  }

  return rc;
}

Result edznchtGetLoadedCheatCount(size_t *loadedCheatCnt) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = ipcPrepareHeader(&c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 9;

  Result rc = serviceIpcDispatch(&g_edznchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;

    struct {
        u64 magic;
        u64 result;
        size_t loadedCheatCnt;
    } *resp;

    serviceIpcParse(&g_edznchtService, &r, sizeof(*resp));
    resp = r.Raw;

    *loadedCheatCnt = resp->loadedCheatCnt;

    rc = resp->result;
  }

  return rc;
}