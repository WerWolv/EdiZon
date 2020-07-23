#pragma once

#include "guis/gui.hpp"
#include "types.h"

#include <vector>
#include <set>
#include <unordered_map>
#include <stdbool.h>
#include <time.h>
#include "helpers/debugger.hpp"
#include "helpers/memory_dump.hpp"

#include "helpers/dmntcht.h"

enum
{
  FORMAT_DEC,
  FORMAT_HEX
} m_searchValueFormat = FORMAT_DEC;

class GuiCheats : public Gui
{

public:
  GuiCheats();
  ~GuiCheats();

  void update();
  void draw();
  void onInput(u32 kdown);
  void onTouch(touchPosition &touch);
  void onGesture(touchPosition startPosition, touchPosition endPosition, bool finish);

private:
  Debugger *m_debugger;
  u8 m_ramBuffer[0x10 * 14] = {0};
  color_t m_memory[1024] = {0};

  u8 m_selectedEntry = 0;
  u8 m_searchValueIndex = 0;
  u32 m_addresslist_offset = 0;

  searchValue_t m_searchValue[2];

  enum
  {
    CHEATS,
    CANDIDATES
  } m_menuLocation = CANDIDATES;
  enum
  {
    SEARCH_NONE,
    SEARCH_TYPE,
    SEARCH_MODE,
    SEARCH_REGION,
    SEARCH_VALUE,
    SEARCH_editRAM,
    SEARCH_POINTER
  } m_searchMenuLocation = SEARCH_NONE;

  searchType_t m_searchType = SEARCH_TYPE_NONE;
  searchMode_t m_searchMode = SEARCH_MODE_NONE;
  searchRegion_t m_searchRegion = SEARCH_REGION_NONE;

  std::vector<MemoryInfo> m_memoryInfo;
  std::vector<MemoryInfo> m_targetmemInfos;

  MemoryDump *m_memoryDump;
  MemoryDump *m_memoryDump1;
  MemoryDump *m_memoryDumpBookmark;
  MemoryDump *m_AttributeDumpBookmark;
  MemoryDump *m_pointeroffsetDump;
  MemoryDump *m_dataDump;
  u64 m_target = 0;
  u64 m_numoffset = 3;
  u64 m_max_source = 200;
  u64 m_max_depth = 2;
  u64 m_max_range = 0x300;
  u64 m_low_main_heap_addr = 0x100000000;
  u64 m_high_main_heap_addr = 0x10000000000;
  u64 m_pointer_found = 0;
  bool m_abort = false;

  std::map<u64, u64> m_frozenAddresses;

  bool m_cheatsPresent = false;
  bool m_sysmodulePresent = false;
  bool m_editRAM = false;
  bool m_nothingchanged = false;

  u64 m_addressSpaceBaseAddr = 0x00;
  u64 m_addressSpaceSize = 0x00;
  u64 m_heapBaseAddr = 0x00;
  u64 m_mainBaseAddr = 0x00;
  u64 m_EditorBaseAddr = 0x00;
  u8 m_addressmod = 0;
  time_t m_Time1;
  struct helperinfo_t
  {
    u64 address;
    u64 size;
    u64 count;
  };
  struct sourceinfo_t
  {
    u64 foffset;
    u64 offset;
  };

#define IS_PTRCODE_START 0x580F0000
#define IS_OFFSET 0x580F1000
#define IS_FINAL 0x780F0000

#define MAX_NUM_SOURCE_POINTER 200 // bound check for debugging;
#define MAX_POINTER_DEPTH 6        // up to 4 seems OK with forward only search took 94s. 215s for big dump
#define MAX_POINTER_RANGE 0x2000
#define MAX_POINTER_TARGET 3
  bool m_forwardonly = false;
  bool m_forwarddump = false; // reduce from 138 to 26
  struct pointer_chain_t
  {
    u64 depth;
    u64 offset[MAX_POINTER_DEPTH + 1]; // offset to address pointed by pointer
    // below is for debugging can consider removing;
    u64 fileoffset[MAX_POINTER_DEPTH + 1]; // offset to the file that has the address where the pointer was stored in this instance for debugging
  };
  struct bookmark_t
  {
    char label[18];
    searchType_t type;
    pointer_chain_t pointer;
    bool heap = true;
    u32 offset = 0;
    bool deleted = false;
  };
  bookmark_t bookmark;
  pointer_chain_t m_hitcount;

  void rebasepointer(); //struct bookmark_t bookmark);
  // bool check_chain(bookmark_t *bookmark, u64 *address);
  // void startpointersearch(u64 address, u64 depth, u64 range, u64 num, MemoryDump **displayDump);
  // void startpointersearch(u64 address, u64 depth, u64 range, u64 num, MemoryDump **displayDump);
  // void pointersearch(u64 targetaddress, MemoryDump **displayDump, MemoryDump **dataDump, pointer_chain_t pointerchain);
  void pointersearch(u64 targetaddress, struct pointer_chain_t pointerchain);
  void pointercheck();
  void startpointersearch(u64 targetaddress); //, MemoryDump **displayDump);
  void searchpointer(u64 address, u64 depth, u64 range, struct pointer_chain_t pointerchain);

  u64 m_heapSize = 0;
  u64 m_mainSize = 0;
  u64 m_heapEnd = 0;
  u64 m_mainend = 0;
  u8 m_buildID[0x20];

  DmntCheatEntry *m_cheats;
  u64 m_cheatCnt;

  void drawSearchRAMMenu();

  void drawEditRAMMenu();
  void drawSearchPointerMenu();
  void searchMemoryAddressesPrimary(Debugger *debugger, searchValue_t searchValue1,
                                    searchValue_t searchValue2, searchType_t searchType,
                                    searchMode_t searchMode, searchRegion_t searchRegion,
                                    MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void searchMemoryAddressesSecondary(Debugger *debugger, searchValue_t searchValue1,
                                      searchValue_t searchValue2, searchType_t searchType,
                                      searchMode_t searchMode, MemoryDump **displayDump);

  void searchMemoryValuesPrimary(Debugger *debugger, searchType_t searchType, searchMode_t searchMode,
                                 searchRegion_t searchRegion, MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void searchMemoryValuesSecondary(Debugger *debugger, searchType_t searchType,
                                   searchMode_t searchMode, searchRegion_t searchRegion,
                                   MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);

  void searchMemoryValuesTertiary(Debugger *debugger, searchValue_t searchValue1,
                                  searchValue_t searchValue2, searchType_t searchType,
                                  searchMode_t searchMode, searchRegion_t searchRegion,
                                  MemoryDump **displayDump, std::vector<MemoryInfo> memInfos);
};
