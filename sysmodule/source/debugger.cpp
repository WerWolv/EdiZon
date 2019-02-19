#include "debugger.hpp"

Debugger::Debugger() {
  pmdmntInitialize();
  pminfoInitialize();
}

Debugger::~Debugger() {
  pminfoExit();
  pmdmntExit();
}

Result Debugger::attachToProcess() {
  u64 pid;
  pmdmntGetApplicationPid(&pid);

  return svcDebugActiveProcess(&m_debugHandle, pid);
}

Result Debugger::detachFromProcess() {
  return svcCloseHandle(m_debugHandle);
}

Result Debugger::continueProcess() {
  Result rc;
  do {
    u8 event[0x40];
    rc = svcGetDebugEvent(event, m_debugHandle);
  } while (R_SUCCEEDED(rc));

  return svcContinueDebugEvent(m_debugHandle, 0, nullptr, 0);
}

Result Debugger::breakProcess() {
  return svcBreakDebugProcess(m_debugHandle);
}

u64 Debugger::getRunningApplicationTID() {
  u64 tid = 0, pid = 0;

  pmdmntGetApplicationPid(&pid);
  pminfoGetTitleId(&tid, pid);
  return tid;
}

u64 Debugger::getRunningApplicationPID() {
  u64 pid = 0;

  pmdmntGetApplicationPid(&pid);
  return pid;
}

u64 Debugger::peekMemory(u64 address) {
  u64 out;
  if (R_FAILED(svcReadDebugProcessMemory(&out, m_debugHandle, address, sizeof(u64)))) {
    attachToProcess();
    svcReadDebugProcessMemory(&out, m_debugHandle, address, sizeof(u64));
  }

  return out;
}

void Debugger::pokeMemory(size_t varSize, u64 address, u64 value) {
  if (R_FAILED(svcWriteDebugProcessMemory(m_debugHandle, &value, address, varSize))) {
    attachToProcess();
    svcWriteDebugProcessMemory(m_debugHandle, &value, address, varSize);
  }
}

void Debugger::readMemory(void *buffer, size_t bufferSize, u64 address) {
  if (R_FAILED(svcReadDebugProcessMemory(buffer, m_debugHandle, address, bufferSize))) {
    attachToProcess();
    svcReadDebugProcessMemory(buffer, m_debugHandle, address, bufferSize);
  }
}

MemoryInfo Debugger::queryMemory(u64 address) {
  MemoryInfo memInfo = { 0 };
  u32 pageInfo;

  svcQueryDebugProcessMemory(&memInfo, &pageInfo, m_debugHandle, address);

  return memInfo;
}