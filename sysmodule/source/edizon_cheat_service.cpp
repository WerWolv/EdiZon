#include "edizon_cheat_service.hpp"

Result EdiZonCheatService::addMemoryFreeze(u64 freezeAddr, u64 freezeValue, u64 valueType) {
  mutexLock(&EdiZonCheatService::g_freezeMutex);
 
  if (g_frozenAddresses.find(freezeAddr) == g_frozenAddresses.end()) {
    g_frozenAddresses.insert(std::make_pair(freezeAddr, (FreezeInfo){ freezeValue, valueType }));

    FILE *file = fopen("/EdiZon/sys.log", "wt");
    fprintf(file, "Added variable: %lx, %lx %lx\n", freezeAddr, freezeValue, valueType);
    fclose(file);
    mutexUnlock(&EdiZonCheatService::g_freezeMutex);

    return 0;
  }

  mutexUnlock(&EdiZonCheatService::g_freezeMutex);

  return 1;
}

Result EdiZonCheatService::removeMemoryFreeze(u64 freezeAddr) {
  mutexLock(&EdiZonCheatService::g_freezeMutex);
  if (g_frozenAddresses.find(freezeAddr) != g_frozenAddresses.end()) {
    g_frozenAddresses.erase(freezeAddr);
    mutexUnlock(&EdiZonCheatService::g_freezeMutex);
    
    return 0;
  }
  
  mutexUnlock(&EdiZonCheatService::g_freezeMutex);

  return 1;
}

Result EdiZonCheatService::updateMemoryFreeze(u64 freezeAddr, u64 newValue) {
  mutexLock(&EdiZonCheatService::g_freezeMutex);
  if (g_frozenAddresses.find(freezeAddr) != g_frozenAddresses.end()) {
    g_frozenAddresses[freezeAddr].freezeValue = newValue;
    mutexUnlock(&EdiZonCheatService::g_freezeMutex);

    return 0;
  }

  mutexUnlock(&EdiZonCheatService::g_freezeMutex);

  return 1;
}


Result EdiZonCheatService::getFrozenMemoryAddresses(Out<size_t> bufferSize, OutBuffer<u64> freezeAddrs) { 
  mutexLock(&EdiZonCheatService::g_freezeMutex);

  if (freezeAddrs.num_elements < g_frozenAddresses.size()) {
    mutexUnlock(&EdiZonCheatService::g_freezeMutex);

    return 1;
  }

  bufferSize.SetValue(g_frozenAddresses.size());

  u32 i = 0;
  for (auto const& [addr, info] : g_frozenAddresses)
    freezeAddrs.buffer[i++] = addr;

  mutexUnlock(&EdiZonCheatService::g_freezeMutex);

  return 0;
}

Result EdiZonCheatService::getFrozenMemoryAddressCount(Out<size_t> frozenAddrCnt) {
  mutexLock(&EdiZonCheatService::g_freezeMutex);

  frozenAddrCnt.SetValue(g_frozenAddresses.size());

  mutexUnlock(&EdiZonCheatService::g_freezeMutex);

  return 0;
}


Result EdiZonCheatService::loadCheat(InBuffer<char> fileName, Out<u64> cheatId) {
  return 0x1337;
}

Result EdiZonCheatService::unloadCheat(u64 cheatId) {
  return 0x1337;
}

Result EdiZonCheatService::GetLoadedCheats(OutBuffer<u64> cheatIds) {
  return 0x1337;
}
  