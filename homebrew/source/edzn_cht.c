#include "edzn_cht.h"

static Service g_edznchtService;
static u64 g_refCnt;

Result edznchtInitialize(void) {
  atomicIncrement64(&g_refCnt);

  if (g_edznchtService.type != ServiceType_Uninitialized)
    return 0;

  return smGetService(&g_edznchtService, "edzn:cht");
}

void edznchtExit(void) {
  if (atomicDecrement64(&g_refCnt) == 0) {
    serviceClose(&g_edznchtService);
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
