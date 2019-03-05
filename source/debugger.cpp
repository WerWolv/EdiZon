#include "debugger.hpp"

extern "C" {
  #include "dmntcht.h"
}

Debugger::Debugger(bool dmntPresent) : m_dmntPresent(dmntPresent) {
  pmdmntInitialize();
  pminfoInitialize();

  pmdmntGetApplicationPid(&m_pid);
  pminfoGetTitleId(&m_tid, m_pid);
}

Debugger::~Debugger() {
  pminfoExit();
  pmdmntExit();
}

Result Debugger::attachToProcess() {
  if (m_dmntPresent) {
    return dmntchtForceOpenCheatProcess();
  } else {
    u64 pid;
    pmdmntGetApplicationPid(&pid);

    return svcDebugActiveProcess(&m_debugHandle, pid);
  }
}

Result Debugger::detachFromProcess() {
  if (m_dmntPresent) 
    return 0;

  return svcCloseHandle(m_debugHandle);
}

Result Debugger::continueProcess() {
  if (m_dmntPresent)
    return 0;

  Result rc;
  do {
    u8 event[0x40];
    rc = svcGetDebugEvent(event, m_debugHandle);
  } while (R_SUCCEEDED(rc));

  return svcContinueDebugEvent(m_debugHandle, 0, nullptr, 0);
}

Result Debugger::breakProcess() {
  if (m_dmntPresent)
    return 0;

  return svcBreakDebugProcess(m_debugHandle);
}

u64 Debugger::getRunningApplicationTID() {
  return m_tid;
}

u64 Debugger::getRunningApplicationPID() {
  return m_pid;
}

u64 Debugger::peekMemory(u64 address) {
  u64 out;
  if (m_dmntPresent)
    dmntchtReadCheatProcessMemory(address, &out, sizeof(u64));
  else
    svcReadDebugProcessMemory(&out, m_debugHandle, address, sizeof(u64));

  return out;
}

void Debugger::pokeMemory(size_t varSize, u64 address, u64 value) {
  if (m_dmntPresent)
    dmntchtWriteCheatProcessMemory(address, &value, varSize);
  else
    svcWriteDebugProcessMemory(m_debugHandle, &value, address, varSize);
}

void Debugger::readMemory(void *buffer, size_t bufferSize, u64 address) {
  if (m_dmntPresent)
    dmntchtReadCheatProcessMemory(address, buffer, bufferSize);
  else
    svcReadDebugProcessMemory(buffer, m_debugHandle, address, bufferSize);}

void Debugger::writeMemory(void *buffer, size_t bufferSize, u64 address) {
  svcWriteDebugProcessMemory(m_debugHandle, buffer, address, bufferSize);
}

MemoryInfo Debugger::queryMemory(u64 address) {
  MemoryInfo memInfo = { 0 };

  if (m_dmntPresent) {
    dmntchtQueryCheatProcessMemory(&memInfo, address);
  } else {
    u32 pageInfo;

    svcQueryDebugProcessMemory(&memInfo, &pageInfo, m_debugHandle, address);
  }

  return memInfo;
}