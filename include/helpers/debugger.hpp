#pragma once

#include <edizon.h>

class Debugger {
public:
  Debugger();
  ~Debugger();

  Result attachToProcess();
  void detatch();
  u64 getRunningApplicationTID();
  u64 getRunningApplicationPID();
  bool m_dmnt = true;
  Result m_rc = 0;
  Handle m_debugHandle = 0; // no action to be taken to attach
  u64 peekMemory(u64 address);
  Result pokeMemory(size_t varSize, u64 address, u64 value);
  MemoryInfo queryMemory(u64 address);
  Result readMemory(void *buffer, size_t bufferSize, u64 address);
  Result writeMemory(void *buffer, size_t bufferSize, u64 address);

private:
  u64 m_tid = 0, m_pid = 0;
};