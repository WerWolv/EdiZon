#include "helpers/debugger.hpp"

#include "helpers/dmntcht.h"

Debugger::Debugger() {
  pmdmntInitialize();
  pminfoInitialize();

  m_pid = 0;
  m_tid = 0;

  pmdmntGetApplicationProcessId(&m_pid);
  pminfoGetProgramId(&m_tid, m_pid);
}

Debugger::~Debugger() {
  pminfoExit();
  pmdmntExit();
}

Result Debugger::attachToProcess() {
  return dmntchtForceOpenCheatProcess();
}

u64 Debugger::getRunningApplicationTID() {
  return m_tid;
}

u64 Debugger::getRunningApplicationPID() {
  return m_pid;
}

u64 Debugger::peekMemory(u64 address) {
  u64 out;
  dmntchtReadCheatProcessMemory(address, &out, sizeof(u64));

  return out;
}

Result Debugger::pokeMemory(size_t varSize, u64 address, u64 value) {
  return dmntchtWriteCheatProcessMemory(address, &value, varSize);
}

Result Debugger::readMemory(void *buffer, size_t bufferSize, u64 address) {
  return dmntchtReadCheatProcessMemory(address, buffer, bufferSize);
}

Result Debugger::writeMemory(void *buffer, size_t bufferSize, u64 address) {
  return dmntchtWriteCheatProcessMemory(address, buffer, bufferSize);
}

MemoryInfo Debugger::queryMemory(u64 address) {
  MemoryInfo memInfo = { 0 };

  dmntchtQueryCheatProcessMemory(&memInfo, address);

  return memInfo;
}