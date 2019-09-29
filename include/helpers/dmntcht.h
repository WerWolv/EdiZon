#pragma once

#include <edizon.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    u64 base;
    u64 size;
} DmntMemoryRegionExtents;

typedef struct {
    u64 process_id;
    u64 title_id;
    DmntMemoryRegionExtents main_nso_extents;
    DmntMemoryRegionExtents heap_extents;
    DmntMemoryRegionExtents alias_extents;
    DmntMemoryRegionExtents address_space_extents;
    u8 main_nso_build_id[0x20];
} DmntCheatProcessMetadata;

typedef struct {
    char readable_name[0x40];
    uint32_t num_opcodes;
    uint32_t opcodes[0x100];
} DmntCheatDefinition;

typedef struct {
    bool enabled;
    uint32_t cheat_id;
    DmntCheatDefinition definition;
} DmntCheatEntry;

typedef struct {
    u64 value;
    u8 width;
} DmntFrozenAddressValue;

typedef struct {
    u64 address;
    DmntFrozenAddressValue value;
} DmntFrozenAddressEntry;

Result dmntchtInitialize(void);
void dmntchtExit(void);
Service* dmntchtGetServiceSession(void);

Result dmntchtHasCheatProcess(bool *out);
Result dmntchtGetCheatProcessEvent(Event *event);
Result dmntchtGetCheatProcessMetadata(DmntCheatProcessMetadata *out_metadata);
Result dmntchtForceOpenCheatProcess(void);

Result dmntchtGetCheatProcessMappingCount(u64 *out_count);
Result dmntchtGetCheatProcessMappings(MemoryInfo *buffer, u64 max_count, u64 offset, u64 *out_count);
Result dmntchtReadCheatProcessMemory(u64 address, void *buffer, size_t size);
Result dmntchtWriteCheatProcessMemory(u64 address, const void *buffer, size_t size);
Result dmntchtQueryCheatProcessMemory(MemoryInfo *mem_info, u64 address);

Result dmntchtGetCheatCount(u64 *out_count);
Result dmntchtGetCheats(DmntCheatEntry *buffer, u64 max_count, u64 offset, u64 *out_count);
Result dmntchtGetCheatById(DmntCheatEntry *out_cheat, u32 cheat_id);
Result dmntchtToggleCheat(u32 cheat_id);
Result dmntchtAddCheat(DmntCheatDefinition *cheat, bool enabled, u32 *out_cheat_id);
Result dmntchtRemoveCheat(u32 cheat_id);

Result dmntchtGetFrozenAddressCount(u64 *out_count);
Result dmntchtGetFrozenAddresses(DmntFrozenAddressEntry *buffer, u64 max_count, u64 offset, u64 *out_count);
Result dmntchtGetFrozenAddress(DmntFrozenAddressEntry *out, u64 address);
Result dmntchtEnableFrozenAddress(u64 address, u64 width, u64 *out_value);
Result dmntchtDisableFrozenAddress(u64 address);

#ifdef __cplusplus
}
#endif