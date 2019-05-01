#include "helpers/debugger.hpp"

extern "C" {
  #include "helpers/dmntcht.h"
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
  return m_tid;
}

u64 Debugger::getRunningApplicationPID() {
  return m_pid;
}

u64 Debugger::peekMemory(u64 address) {
  u64 out;
  svcReadDebugProcessMemory(&out, m_debugHandle, address, sizeof(u64));

  return out;
}

void Debugger::pokeMemory(size_t varSize, u64 address, u64 value) {
  svcWriteDebugProcessMemory(m_debugHandle, &value, address, varSize);
}

void Debugger::readMemory(void *buffer, size_t bufferSize, u64 address) {
  svcReadDebugProcessMemory(buffer, m_debugHandle, address, bufferSize);}

void Debugger::writeMemory(void *buffer, size_t bufferSize, u64 address) {
  svcWriteDebugProcessMemory(m_debugHandle, buffer, address, bufferSize);
}

MemoryInfo Debugger::queryMemory(u64 address) {
  MemoryInfo memInfo = { 0 };
  u32 pageInfo;

  svcQueryDebugProcessMemory(&memInfo, &pageInfo, m_debugHandle, address);

  return memInfo;
}