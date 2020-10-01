#pragma once

#include "types.h"

#include <string>
#include <vector>
extern bool print_details;
// extern bool m_compress = false;
class MemoryDump
{
public:
	MemoryDump(std::string filePath, DumpType dumpType, bool discardFile);
	~MemoryDump();

	void setBaseAddresses(u64 addrSpaceBase, u64 heapBase, u64 mainBase, u64 heapSize, u64 mainSize);
	void setSearchParams(searchType_t searchDataType, searchMode_t searchMode, searchRegion_t searchRegion, searchValue_t searchValue1, searchValue_t searchValue2);
	void setPointerSearchParams(u64 max_depth, u64 numoffset, u64 maxrange, u8 buildID[0x20]);
	void addData(u8 *buffer, size_t bufferSize);
	size_t size();
	void clear();

	int getData(u64 addr, void *buffer, size_t bufferSize);
	int putData(u64 addr, void *buffer, size_t bufferSize);
	u8 operator[](u64 index);

	data_header_t getDumpInfo();

	void setDumpType(enum DumpType dumpType);
	void flushBuffer();
	bool m_compress = false;

private:
	FILE *m_dumpFile;
	std::string m_filePath;
	std::vector<u8> m_data;

	data_header_t m_dataHeader;

	bool isFileOpen();
	void writeHeader();
};