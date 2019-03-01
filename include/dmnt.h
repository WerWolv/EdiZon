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

Result dmntDmntInitialize(void);
void dmntExit(void);
Service* dmntGetService(void);

Result dmntHasCheatProcess(bool *out);
Result dmntGetCheatProcessEvent(Event *event);
Result dmntGetCheatProcessMetadata(CheatProcessMetadata *out_metadata);

Result dmntGetCheatProcessMappingCount(u64 *out_count);
Result dmntGetCheatProcessMapping(MemoryInfo *buffer, u64 buffer_size, u64 offset, u64 *out_count);
Result dmntReadCheatProcessMemory(u8 *buffer, u64 buffer_size, u64 address, u64 *size);
Result dmntWriteCheatProcessMemory(u8 *buffer, u64 buffer_size, u64 address, u64 *size);

Result dmntGetCheatCount(u64 *out_count);
Result dmntGetCheats(CheatEntry *buffer, u64 buffer_size, u64 offset, u64 *out_count);
Result dmntGetCheatById(CheatEntry *buffer, u32 cheat_id);
Result dmntToggleCheat(u32 cheat_id);
Result dmntAddCheat(CheatDefinition *buffer, bool enabled, u32 *out_cheat_id);
Result dmntRemoveCheat(u32 cheat_id);

Result dmntGetFrozenAddressCount(u64 *out_count);
Result dmntGetFrozenAddresses(uintptr_t *buffer, u64 buffer_size, u64 offset, u64 *out_count);
Result dmntToggleAddressFrozen(uintptr_t address);