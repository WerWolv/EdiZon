#pragma once
#include <switch.h>

#include <stratosphere.hpp>

#include <map>

enum EdiZonCheatServiceCmd {
    EdiZonCheat_Cmd_AddMemoryFreeze = 0,
    EdiZonCheat_Cmd_RemoveMemoryFreeze = 1,
    EdiZonCheat_Cmd_UpdateMemoryFreeze = 2,
    EdiZonCheat_Cmd_GetFrozenMemoryAddresses = 3,
    EdiZonCheat_Cmd_GetFrozenMemoryAddressCount = 4,
    EdiZonCheat_Cmd_LoadCheat = 5,
    EdiZonCheat_Cmd_UnloadCheat = 6,
    EdiZonCheat_Cmd_GetLoadedCheats = 7
};

struct FreezeInfo {
  u64 freezeValue;
  u64 valueType;
};

class EdiZonCheatService final : public IServiceObject {
private:
  Result addMemoryFreeze(u64 freezeAddr, u64 freezeValue, u64 valueType);
  Result removeMemoryFreeze(u64 freezeAddr);
  Result updateMemoryFreeze(u64 freezeAddr, u64 newValue);
  Result getFrozenMemoryAddresses(Out<size_t> bufferSize, OutBuffer<u64> freezeAddrs);
  Result getFrozenMemoryAddressCount(Out<size_t> frozenAddrCnt);

  Result loadCheat(InBuffer<char> fileName, Out<u64> cheatId);
  Result unloadCheat(u64 cheatId);
  Result GetLoadedCheats(OutBuffer<u64> cheatIds);

public:
  static inline std::map<u64, FreezeInfo> g_frozenAddresses;
  static inline Mutex g_freezeMutex;

  DEFINE_SERVICE_DISPATCH_TABLE {
    MakeServiceCommandMeta<EdiZonCheat_Cmd_AddMemoryFreeze, &EdiZonCheatService::addMemoryFreeze>(),
    MakeServiceCommandMeta<EdiZonCheat_Cmd_RemoveMemoryFreeze, &EdiZonCheatService::removeMemoryFreeze>(),
    MakeServiceCommandMeta<EdiZonCheat_Cmd_UpdateMemoryFreeze, &EdiZonCheatService::updateMemoryFreeze>(),
    MakeServiceCommandMeta<EdiZonCheat_Cmd_GetFrozenMemoryAddresses, &EdiZonCheatService::getFrozenMemoryAddresses>(),
    MakeServiceCommandMeta<EdiZonCheat_Cmd_GetFrozenMemoryAddressCount, &EdiZonCheatService::getFrozenMemoryAddressCount>(),

    MakeServiceCommandMeta<EdiZonCheat_Cmd_LoadCheat, &EdiZonCheatService::loadCheat>(),
    MakeServiceCommandMeta<EdiZonCheat_Cmd_UnloadCheat, &EdiZonCheatService::unloadCheat>(),
    MakeServiceCommandMeta<EdiZonCheat_Cmd_GetLoadedCheats, &EdiZonCheatService::GetLoadedCheats>()
  };
};