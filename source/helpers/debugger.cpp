#include "helpers/debugger.hpp"

#include "helpers/dmntcht.h"

Debugger::Debugger() {
  pmdmntInitialize();
  pminfoInitialize();

  m_pid = 0;
  m_tid = 0;

  pmdmntGetApplicationPid(&m_pid);
  pminfoGetTitleId(&m_tid, m_pid);
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

void Debugger::pokeMemory(size_t varSize, u64 address, u64 value) {
  dmntchtWriteCheatProcessMemory(address, &value, varSize);
}

void Debugger::readMemory(void *buffer, size_t bufferSize, u64 address) {
  dmntchtReadCheatProcessMemory(address, buffer, bufferSize);
}

void Debugger::writeMemory(void *buffer, size_t bufferSize, u64 address) {
  dmntchtWriteCheatProcessMemory(address, buffer, bufferSize);
}

MemoryInfo Debugger::queryMemory(u64 address) {
  MemoryInfo memInfo = { 0 };

  dmntchtQueryCheatProcessMemory(&memInfo, address);

  return memInfo;
}