#pragma once

#include <switch.h>

extern "C" {
  #include "types.h"
}

#include <vector>
#include <string>
#include <fstream>

class MemoryDump {
public:
  explicit MemoryDump(std::string fileLocation, u64& heapBase, searchValue_t& searchValue1, searchValue_t& searchValue2, searchType_t& searchType, searchRegion_t& searchRegion, bool discard);
  ~MemoryDump();

  void pushAddress(ramAddr_t addr);
  void flush();
  ramAddr_t getAddress(u32 index);
  std::vector<ramAddr_t>::iterator getAddressIterator();

  void clearAddresses();
  void setSearchValue(searchValue_t searchValue);

  ramAddr_t operator[](u32 index);
  void initFile(bool discard);

  size_t size();
  int clear();

  inline searchValue_t getSearchValue1() {
    return m_searchValue1;
  }

  inline searchValue_t getSearchValue2() {
    return m_searchValue2;
  }

  inline searchType_t getSearchType() {
    return m_searchType;
  }

  inline searchRegion_t getSearchRegion() {
    return m_searchRegion;
  }

  inline u64 getHeapBase() {
    return m_heapBase;
  }

private:
  std::vector<ramAddr_t> m_addresses;
  size_t m_addressCnt;
  std::fstream m_dumpFile;

  std::string m_fileLocation;
  u64 m_heapBase = 0x00;

  searchValue_t m_searchValue1, m_searchValue2;
  searchType_t m_searchType;
  searchRegion_t m_searchRegion;
};