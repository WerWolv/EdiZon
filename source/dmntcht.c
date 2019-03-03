#include "dmntcht.h"

static Service g_dmntchtService;
static u64 g_refCnt;

Result dmntchtInitialize(void) {
  atomicIncrement64(&g_refCnt);

  if (serviceIsActive(&g_dmntchtService))
    return 0;

  return smGetService(&g_dmntchtService, "dmnt:cht");
}

void dmntchtExit(void) {
  if (atomicIncrement64(&g_refCnt) == 0)
    serviceClose(&g_dmntchtService);
}

Service* dmntchtGetService(void) {
  return &g_dmntchtService;
}


Result dmntchtHasCheatProcess(bool *out) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65000;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
        bool out;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
    *out = resp->out;
  }

  return rc;
}

Result dmntchtGetCheatProcessEvent(Event *event) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65001;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;

    if (R_SUCCEEDED(rc)) {
      eventLoadRemote(event, r.Handles[0], true);
    }
  }

  return rc;
}

Result dmntchtGetCheatProcessMetadata(CheatProcessMetadata *out_metadata) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65002;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
        CheatProcessMetadata metadata;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
    *out_metadata = resp->metadata;
  }

  return rc;
}

Result dmntchtGetCheatProcessMappingCount(u64 *out_count) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65100;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
        u64 count;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
    *out_count = resp->count;
  }

  return rc;
}

Result dmntchtGetCheatProcessMapping(MemoryInfo *buffer, u64 buffer_size, u64 offset, u64 *out_count) {
  IpcCommand c;
  ipcInitialize(&c);
  ipcAddRecvBuffer(&c, buffer, buffer_size * sizeof(MemoryInfo), 0);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 offset;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65101;
  raw->offset = offset;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
        u64 count;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
    *out_count = resp->count;
  }

  return rc;
}

Result dmntchtReadCheatProcessMemory(u8 *buffer, u64 buffer_size, u64 address, u64 size) {
  IpcCommand c;
  ipcInitialize(&c);
  ipcAddRecvBuffer(&c, buffer, buffer_size * sizeof(u8), 0);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 address;
    u64 size;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65102;
  raw->address = address;
  raw->size = size;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}

Result dmntchtWriteCheatProcessMemory(u8 *buffer, u64 buffer_size, u64 address, u64 size) {
  IpcCommand c;
  ipcInitialize(&c);
  ipcAddSendBuffer(&c, buffer, buffer_size * sizeof(u8), 0);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 address;
    u64 size;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65103;
  raw->address = address;
  raw->size = size;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}

Result dmntchtGetCheatCount(u64 *out_count) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65200;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
        u64 count;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
    *out_count = resp->count;
  }

  return rc;
}

Result dmntchtGetCheats(CheatEntry *buffer, u64 buffer_size, u64 offset, u64 *out_count) {
  IpcCommand c;
  ipcInitialize(&c);
  ipcAddRecvBuffer(&c, buffer, buffer_size * sizeof(CheatEntry), 0);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 offset;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65201;
  raw->offset = offset;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
        u64 count;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
    *out_count = resp->count;
  }

  return rc;
}

Result dmntchtGetCheatById(CheatEntry *buffer, u32 cheat_id) {
  IpcCommand c;
  ipcInitialize(&c);
  ipcAddRecvBuffer(&c, buffer, sizeof(CheatEntry), 0);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 cheat_id;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65202;
  raw->cheat_id = cheat_id;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
        u64 count;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}

Result dmntchtToggleCheat(u32 cheat_id) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 cheat_id;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65203;
  raw->cheat_id = cheat_id;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}

Result dmntchtAddCheat(CheatDefinition *buffer, bool enabled, u32 *out_cheat_id) {
  IpcCommand c;
  ipcInitialize(&c);
  ipcAddSendBuffer(&c, buffer, sizeof(CheatDefinition), 0);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 enabled;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65204;
  raw->enabled = enabled;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
        u64 cheat_id;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;
    *out_cheat_id = resp->cheat_id;

    rc = resp->result;
  }

  return rc;
}

Result dmntchtRemoveCheat(u32 cheat_id) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 cheat_id;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65205;
  raw->cheat_id = cheat_id;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}


Result dmntchtGetFrozenAddressCount(u64 *out_count) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65300;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
        u64 count;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
    *out_count = resp->count;
  }

  return rc;
}

Result dmntchtGetFrozenAddresses(uintptr_t *buffer, u64 buffer_size, u64 offset, u64 *out_count) {
  IpcCommand c;
  ipcInitialize(&c);
  ipcAddRecvBuffer(&c, buffer, buffer_size * sizeof(uintptr_t), 0);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 offset;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65301;
  raw->offset = offset;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
        u64 count;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;
    *out_count = resp->count;

    rc = resp->result;
  }

  return rc;
}

Result dmntchtToggleAddressFrozen(uintptr_t address) {
  IpcCommand c;
  ipcInitialize(&c);

  struct {
    u64 magic;
    u64 cmd_id;
    u64 address;
  } *raw;

  raw = serviceIpcPrepareHeader(&g_dmntchtService, &c, sizeof(*raw));

  raw->magic = SFCI_MAGIC;
  raw->cmd_id = 65302;
  raw->address = address;

  Result rc = serviceIpcDispatch(&g_dmntchtService);

  if (R_SUCCEEDED(rc)) {
    IpcParsedCommand r;
    struct {
        u64 magic;
        u64 result;
    } *resp;

    serviceIpcParse(&g_dmntchtService, &r, sizeof(*resp));
    resp = r.Raw;

    rc = resp->result;
  }

  return rc;
}