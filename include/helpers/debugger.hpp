#pragma once

#include <edizon.h>

class Debugger {
public:
  Debugger();
  ~Debugger();

  Result attachToProcess();

  u64 getRunningApplicationTID();
  u64 getRunningApplicationPID();

  u64 peekMemory(u64 address);
  Result pokeMemory(size_t varSize, u64 address, u64 value);
  MemoryInfo queryMemory(u64 address);
  Result readMemory(void *buffer, size_t bufferSize, u64 address);
  Result writeMemory(void *buffer, size_t bufferSize, u64 address);

  template <typename T>
  Result WriteMemory(u64 address, const T &value) {
    return Debugger::writeMemory((void *)&value, sizeof(T), address);
  }

private:
  Handle m_debugHandle;
  u64 m_tid = 0, m_pid = 0;
};