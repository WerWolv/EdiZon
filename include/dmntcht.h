#pragma once

#include <switch.h>

typedef struct {
    u64 base;
    u64 size;
} MemoryRegionExtents;

typedef struct {
    u64 process_id;
    MemoryRegionExtents main_nso_extents;
    MemoryRegionExtents heap_extents;
    MemoryRegionExtents alias_extents;
    MemoryRegionExtents address_space_extents;
    u8 main_nso_build_id[0x20];
} CheatProcessMetadata;

typedef struct {
    char readable_name[0x40];
    uint32_t num_opcodes;
    uint32_t opcodes[0x100];
} CheatDefinition;

typedef struct {
    bool enabled;
    uint32_t cheat_id;
    CheatDefinition definition;
} CheatEntry;

Result dmntchtInitialize(void);
void dmntchtExit(void);
Service* dmntchtGetService(void);

Result dmntchtHasCheatProcess(bool *out);
Result dmntchtGetCheatProcessEvent(Event *event);
Result dmntchtGetCheatProcessMetadata(CheatProcessMetadata *out_metadata);

Result dmntchtGetCheatProcessMappingCount(u64 *out_count);
Result dmntchtGetCheatProcessMapping(MemoryInfo *buffer, u64 buffer_size, u64 offset, u64 *out_count);
Result dmntchtReadCheatProcessMemory(u8 *buffer, u64 buffer_size, u64 address, u64 *size);
Result dmntchtWriteCheatProcessMemory(u8 *buffer, u64 buffer_size, u64 address, u64 *size);

Result dmntchtGetCheatCount(u64 *out_count);
Result dmntchtGetCheats(CheatEntry *buffer, u64 buffer_size, u64 offset, u64 *out_count);
Result dmntchtGetCheatById(CheatEntry *buffer, u32 cheat_id);
Result dmntchtToggleCheat(u32 cheat_id);
Result dmntchtAddCheat(CheatDefinition *buffer, bool enabled, u32 *out_cheat_id);
Result dmntchtRemoveCheat(u32 cheat_id);

Result dmntchtGetFrozenAddressCount(u64 *out_count);
Result dmntchtGetFrozenAddresses(uintptr_t *buffer, u64 buffer_size, u64 offset, u64 *out_count);
Result dmntchtToggleAddressFrozen(uintptr_t address);