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


Result EdiZonCheatService::loadCheat(InBuffer<char> fileName, InBuffer<char> cheatName) {
  Result ret = 1;

  if (EdiZonCheatService::g_cheatScripts.find(fileName.buffer) == EdiZonCheatService::g_cheatScripts.end())
    ret = 0;
  else if (EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames.find(cheatName.buffer) == EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames.end())
    ret = 0;

  mutexLock(&EdiZonCheatService::g_freezeMutex);
  EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames[cheatName.buffer] = false;
  mutexUnlock(&EdiZonCheatService::g_freezeMutex);

  return ret;
}

Result EdiZonCheatService::unloadCheat(InBuffer<char> fileName, InBuffer<char> cheatName) {
  if (EdiZonCheatService::g_cheatScripts.find(fileName.buffer) == EdiZonCheatService::g_cheatScripts.end())
    return 1;
  else if (EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames.find(cheatName.buffer) == EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames.end())
    return 1;

  mutexLock(&EdiZonCheatService::g_freezeMutex);

  EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames.erase(cheatName.buffer);
  if (EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames.empty())
    EdiZonCheatService::g_cheatScripts.erase(fileName.buffer);

  mutexUnlock(&EdiZonCheatService::g_freezeMutex);

  return 0;
}

Result EdiZonCheatService::activateCheat(InBuffer<char> fileName, InBuffer<char> cheatName, bool activated) {

  if (EdiZonCheatService::g_cheatScripts.find(fileName.buffer) == EdiZonCheatService::g_cheatScripts.end())
    return 1;
  else if (EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames.find(cheatName.buffer) == EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames.end())
    return 1;

  if (EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames[cheatName.buffer] == activated)
    return 2;

  mutexLock(&EdiZonCheatService::g_freezeMutex);
  EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames[cheatName.buffer] = activated;
  mutexUnlock(&EdiZonCheatService::g_freezeMutex);

  return 0;
}

Result EdiZonCheatService::getLoadedCheats(InBuffer<char> fileName, Out<size_t> bufferSize, OutBuffer<bool> enabled) {
  if (EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames.size() > enabled.num_elements)
    return 1;
  
  mutexLock(&EdiZonCheatService::g_freezeMutex);

  u32 i = 0;
  for (auto const& [cheatName, en] : EdiZonCheatService::g_cheatScripts[fileName.buffer].cheatNames)
    enabled.buffer[i++] = en;

  mutexUnlock(&EdiZonCheatService::g_freezeMutex);

  bufferSize.SetValue(i);

  return 0;
}
  