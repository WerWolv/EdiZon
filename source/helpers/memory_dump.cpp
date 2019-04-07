#include "helpers/memory_dump.hpp"

#include <iterator>
#include <cstring>

MemoryDump::MemoryDump(std::string fileLocation, u64 heapBase, searchValue_t searchValue, searchType_t searchType, searchRegion_t searchRegion, bool discard) 
 : m_fileLocation(fileLocation), m_heapBase(heapBase), m_searchValue(searchValue), m_searchType(searchType), m_searchRegion(searchRegion)
{
  s64 dumpFileSize = 0;
  
  m_addressCnt = 0;
  
  if (discard) {
    m_dumpFile.open(fileLocation, std::ios::out);
    m_dumpFile.close();
  }

  m_dumpFile.open(fileLocation, std::ios::in | std::ios::out | std::ios::ate);
  dumpFileSize = m_dumpFile.tellg();
  printf("%d\n", dumpFileSize);
  if (dumpFileSize == 0)
    MemoryDump::flush();
  else {
    m_dumpFile.seekg(0, m_dumpFile.beg);

    m_dumpFile.read((char*)&m_addressCnt, sizeof(size_t));
    m_dumpFile.read((char*)&m_searchType, sizeof(m_searchType));
    m_dumpFile.read((char*)&m_searchRegion, sizeof(m_searchRegion));
    m_dumpFile.read((char*)&m_searchValue, sizeof(m_searchValue));
    m_dumpFile.read((char*)&m_heapBase, sizeof(m_heapBase));
  }
  m_addresses.reserve(0x100000);
}

MemoryDump::~MemoryDump() {
  MemoryDump::flush();
  m_dumpFile.close();
}


void MemoryDump::pushAddress(ramAddr_t addr) {
  m_addresses.push_back(addr);
  m_addressCnt++;

  if (m_addresses.size() >= 0x100000)
    MemoryDump::flush();
}

void MemoryDump::flush() {
  m_dumpFile.seekg(0, m_dumpFile.beg);

  if (m_addressCnt == 0) {
    m_dumpFile.write((char*)&m_addressCnt, sizeof(size_t));
    m_dumpFile.write((char*)&m_searchType, sizeof(m_searchType));
    m_dumpFile.write((char*)&m_searchRegion, sizeof(m_searchRegion));
    m_dumpFile.write((char*)&m_searchValue, sizeof(m_searchValue));
    m_dumpFile.write((char*)&m_heapBase, sizeof(m_heapBase));
    m_dumpFile.flush();
  } else {
    m_dumpFile.write((char*)&m_addressCnt, sizeof(size_t));
    m_dumpFile.seekg(0, m_dumpFile.end);
    for (ramAddr_t &addr : m_addresses)
      m_dumpFile.write((char*)&addr, sizeof(ramAddr_t));

    m_dumpFile.flush();
    m_addresses.clear();
  }
}

ramAddr_t MemoryDump::getAddress(u32 index) {
  if (MemoryDump::size() == 0) return { 0 };

  if (m_addresses.size() > 0)
    MemoryDump::flush();

  ramAddr_t addr = { 0 };
  m_dumpFile.seekg(0x20 + sizeof(ramAddr_t) * index);
  m_dumpFile.read((char*)&addr, sizeof(ramAddr_t));
  return addr;
}

ramAddr_t MemoryDump::operator[](u32 index) {
  return MemoryDump::getAddress(index);
}

std::vector<ramAddr_t>::iterator MemoryDump::getAddressIterator() {
  return m_addresses.begin();
}

void MemoryDump::clearAddresses() {
  char buffer[0x20];

  m_dumpFile.seekg(0, m_dumpFile.beg);
  m_dumpFile.read(buffer, 0x20);
  std::memset(buffer, 0x00, 8);

  m_dumpFile.close();
  remove(m_fileLocation.c_str());
  m_dumpFile.open(m_fileLocation, std::ofstream::out);
  m_dumpFile.close();
  m_dumpFile.open(m_fileLocation, std::ios::in | std::ios::out | std::ios::ate);

  m_dumpFile.seekg(0, m_dumpFile.beg);
  m_dumpFile.write(buffer, 0x20);

  m_addresses.clear();
  m_addressCnt = 0;
}

void MemoryDump::setSearchValue(searchValue_t searchValue) {
  m_dumpFile.seekg(0x10, m_dumpFile.beg);
  m_dumpFile.write((char*)&searchValue, sizeof(searchValue_t));
  m_dumpFile.flush();
}


s64 MemoryDump::size() {
  return m_addressCnt;
}

void MemoryDump::clear() {
  std::remove(m_fileLocation.c_str());
  m_addresses.clear();
  m_addressCnt = 0;
}