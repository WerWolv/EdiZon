#pragma once

#include <switch.h>

Result edznchtInitialize();
void edznchtExit();

Result edznchtAddMemoryFreeze(u64 freezeAddr, u64 freezeValue, u64 valueType);
Result edznchtRemoveMemoryFreeze(u64 freezeAddr);
Result edznchtUpdateMemoryFreeze(u64 freezeAddr, u64 newValue);
Result edznchtGetFrozenMemoryAddresses(u64 *buffer, size_t bufferSize, size_t *addrsRead);
Result edznchtGetFrozenMemoryAddressCount(size_t *frozenAddrCnt);
